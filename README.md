# AeroSleep

A minimalist voice-activated system power management daemon that implements real-time audio stream processing for hands-free OS control.

## Architecture

AeroSleep operates as a continuous event loop that interfaces with three primary subsystems:

1. **Audio Capture Layer** (PyAudio): Low-level microphone stream management
2. **Speech Recognition Pipeline** (Google Web Speech API): Cloud-based acoustic model inference
3. **OS Command Interface**: Platform-specific system call execution

The core design philosophy prioritizes reliability over feature richness—the system does one thing well: monitor for a specific wake phrase and execute a graceful shutdown sequence.

## Technical Implementation

### Audio Processing Pipeline

The recognizer operates with dynamic energy thresholding, which continuously adapts to ambient noise characteristics:

```python
recognizer.dynamic_energy_threshold = True
```

This adaptive mechanism solves the cold-start problem inherent in fixed-threshold systems, where initial calibration in quiet environments fails catastrophically when deployed in noisy contexts. The energy threshold auto-adjusts based on a rolling window of ambient audio levels, providing robust performance across diverse acoustic environments.

### Recognition Loop

The core listening loop implements a blocking architecture:

```python
while True:
    audio = recognizer.listen(source)
    # Process audio chunk
```

This design trades CPU efficiency for simplicity—the thread blocks on `listen()`, eliminating complex async coordination. For a single-purpose daemon, this is architecturally appropriate. The recognizer buffers audio until silence detection triggers, then sends the complete utterance to the recognition backend.

### Network Transport

Audio is serialized to FLAC format (lossless compression) and transmitted via HTTPS to Google's speech API. The system handles three failure modes:

- **Network timeout**: Typically indicates connectivity issues
- **Recognition failure**: Unintelligible speech or excessive noise
- **API errors**: Service unavailability or rate limiting

All exceptions are caught and logged non-fatally—the daemon continues monitoring.

### Command Execution Safety

A 5-second countdown provides a critical safety window:

```python
time.sleep(5)  # Grace period for user intervention
os.system("shutdown /s /t 0")
```

This delay allows users to abort via Ctrl+C if the recognition was spurious (e.g., TV dialogue, conversation fragments). Without this buffer, false positives would be catastrophic.

The `/t 0` flag executes immediate shutdown post-delay. On Windows, `shutdown /a` can abort if caught within the grace period.

## Cross-Platform Considerations

Current implementation targets Windows (`shutdown /s`). Extension to POSIX systems requires:

```python
if platform.system() == "Windows":
    os.system("shutdown /s /t 0")
elif platform.system() == "Linux":
    os.system("sudo systemctl poweroff")
elif platform.system() == "Darwin":  # macOS
    os.system("sudo shutdown -h now")
```

Note: POSIX variants require sudo privileges, necessitating either passwordless sudo configuration or a privileged daemon architecture.

## Dependencies

- **Python 3.7+**: f-string support, type hints
- **PyAudio**: PortAudio bindings for cross-platform audio I/O
- **SpeechRecognition**: Abstraction layer over multiple ASR backends

### Installation

```bash
# Clone repository
git clone https://github.com/Eamon2009/Shutdown_by_voice.git
cd voice-system-control

# Install Python dependencies
pip install -r requirements.txt

# Linux-specific: Install PortAudio system library
sudo apt-get install python3-pyaudio portaudio19-dev
```

## Usage

```bash
python voice_monitor.py
```

Output:
```
--- Voice Control Active ---
Listening...
```

Speak clearly: **"Shutdown system"**

The system logs detection and initiates the countdown sequence.

**Interrupt**: Press `Ctrl+C` to terminate the daemon gracefully.

## Limitations & Design Tradeoffs

### 1. Cloud Dependency
Google's API requires internet connectivity. Offline alternatives (Vosk, PocketSphinx) sacrifice accuracy for autonomy. For a critical system command, accuracy is non-negotiable.

### 2. Single Wake Phrase
Hardcoded to "shutdown system". Extending to multiple commands requires a state machine:
```
Listen → Detect wake word → Parse command → Execute
```

### 3. No Speaker Verification
Any voice can trigger shutdown. Production systems should implement voice biometrics or require a spoken PIN.

### 4. Blocking Architecture
Single-threaded design limits extensibility. Async refactor would enable parallel command queuing but adds complexity.

## Security Considerations

**This tool executes privileged system commands based on audio input.** Threat model:

- **Physical access**: Attacker with microphone proximity can trigger shutdown
- **Audio injection**: Malicious media playback (TV, YouTube) could contain wake phrase
- **Denial of service**: Repeated shutdowns in multi-user environments
---

**License**: MIT  

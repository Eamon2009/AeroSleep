# AeroSleep

A minimalist voice-activated system power management daemon that implements real-time audio stream processing for hands-free OS control.

### Recognition Loop

The core listening loop implements a blocking architecture:

```python
while True:
    audio = recognizer.listen(source)
    # Process audio chunk
```

This design trades CPU efficiency for simplicity—the thread blocks on `listen()`, eliminating complex async coordination. For a single-purpose daemon, this is architecturally appropriate. The recognizer buffers audio until silence detection triggers, then sends the complete utterance to the recognition backend.

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
cd AeroSleep

# Install Python dependencies
pip install -r requirements.txt

# Linux-specific: Install PortAudio system library
sudo apt-get install python3-pyaudio portaudio19-dev
```

## Usage

```bash
cd src
python main.py
```

Output:
```
--- Voice Control Active ---
Listening...
```

Speak clearly: **"Shutdown system"**

The system logs detection and initiates the countdown sequence.

**Interrupt**: Press `Ctrl+C` to terminate the daemon gracefully.

## Security Considerations

**This tool executes privileged system commands based on audio input.** Threat model:

- **Physical access**: Attacker with microphone proximity can trigger shutdown
- **Audio injection**: Malicious media playback (TV, YouTube) could contain wake phrase
- **Denial of service**: Repeated shutdowns in multi-user environments
---

**License**: MIT  

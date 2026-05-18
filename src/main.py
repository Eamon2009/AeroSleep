import speech_recognition as sr
import os
import platform
import time

def shutdown_pc():
    """Triggers the system shutdown command."""
    current_os = platform.system()
    if current_os == "Windows":
        os.system("shutdown /s /t 5") # 5-second delay to allow for a final message

def start_voice_monitor():
    recognizer = sr.Recognizer()
    # Adjusting sensitivity for better accuracy
    recognizer.dynamic_energy_threshold = True 
    
    print("--- Voice Control Active ---")
    print("Waiting for command: 'shutdown system'...")

    while True:
        with sr.Microphone() as source:
            try:
                # Short timeout to keep the loop responsive
                audio = recognizer.listen(source, timeout=None, phrase_time_limit=3)
                
                # Convert speech to text
                command = recognizer.recognize_google(audio).lower()
                print(f"Detected: {command}")

                if "shutdown system" in command:
                    print("Confirmed. Shutting down the PC in 5 seconds...")
                    shutdown_pc()
                    break # Exit the loop
                
            except sr.WaitTimeoutError:
                continue # No speech detected, keep waiting
            except sr.UnknownValueError:
                continue # Speech was blurry, keep waiting
            except sr.RequestError:
                print("Network error. Please check your internet connection.")
                time.sleep(5)
            except KeyboardInterrupt:
                print("Program stopped by user.")
                break

if __name__ == "__main__":
    start_voice_monitor()
    print("shutdown your system sir")
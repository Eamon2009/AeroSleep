/**
 * AeroSleep - Text Command System Controller
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#define PLATFORM "Windows"
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#define PLATFORM "POSIX"
#endif

#define MAX_INPUT 256
#define SAFETY_DELAY_SEC 5

// Global flag for graceful termination
static volatile sig_atomic_t running = 1;

/**
 * Signal handler for SIGINT (Ctrl+C)
 */
void handle_interrupt(int sig)
{
      (void)sig; // Suppress unused parameter warning
      running = 0;
      printf("\n[!] Interrupt received. Shutting down monitor...\n");
}

/**
 * Convert string to lowercase in-place
 */
void to_lowercase(char *str)
{
      for (int i = 0; str[i]; i++)
      {
            str[i] = tolower((unsigned char)str[i]);
      }
}

/**
 * Remove trailing newline and whitespace
 */
void trim_input(char *str)
{
      size_t len = strlen(str);
      while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r' ||
                         str[len - 1] == ' ' || str[len - 1] == '\t'))
      {
            str[--len] = '\0';
      }
}

/**
 * Execute platform-specific shutdown command
 */
int execute_shutdown(void)
{
      int status;

#ifdef _WIN32
      // Windows: shutdown with 5-second delay
      status = system("shutdown /s /t 5");
      if (status == 0)
      {
            printf("[+] Shutdown initiated (5-second Windows delay)\n");
      }
      else
      {
            fprintf(stderr, "[!] Failed to execute shutdown command (code: %d)\n", status);
            return -1;
      }
#elif defined(__linux__)
      // Linux: immediate poweroff (requires sudo privileges)
      printf("[*] Executing: sudo systemctl poweroff\n");
      status = system("sudo systemctl poweroff");
      if (status != 0)
      {
            fprintf(stderr, "[!] Shutdown failed. Ensure sudo privileges are configured.\n");
            return -1;
      }
#elif defined(__APPLE__)
      // macOS: immediate halt
      printf("[*] Executing: sudo shutdown -h now\n");
      status = system("sudo shutdown -h now");
      if (status != 0)
      {
            fprintf(stderr, "[!] Shutdown failed. Ensure sudo privileges are configured.\n");
            return -1;
      }
#else
      fprintf(stderr, "[!] Unsupported platform\n");
      return -1;
#endif

      return 0;
}

/**
 * Main monitoring loop
 */
void start_text_monitor(void)
{
      char input[MAX_INPUT];

      printf("===========================================\n");
      printf("    AeroSleep Text Monitor [%s]\n", PLATFORM);
      printf("===========================================\n");
      printf("Type 'shutdown system' to initiate poweroff\n");
      printf("Press Ctrl+C to exit\n");
      printf("-------------------------------------------\n\n");

      while (running)
      {
            printf(">>> ");
            fflush(stdout);

            // Read line from stdin with timeout handling
            if (fgets(input, sizeof(input), stdin) == NULL)
            {
                  if (feof(stdin))
                  {
                        printf("\n[*] EOF detected. Exiting...\n");
                        break;
                  }
                  if (!running)
                        break; // Interrupted
                  continue;
            }

            // Sanitize input
            trim_input(input);
            to_lowercase(input);

            // Empty input, continue
            if (strlen(input) == 0)
            {
                  continue;
            }

            printf("[*] Detected: \"%s\"\n", input);

            // Check for shutdown command
            if (strstr(input, "shutdown system") != NULL)
            {
                  printf("\n[!] SHUTDOWN COMMAND CONFIRMED\n");
                  printf("[!] System will power off in %d seconds...\n", SAFETY_DELAY_SEC);
                  printf("[!] Press Ctrl+C NOW to abort!\n\n");

                  // Safety countdown
                  for (int i = SAFETY_DELAY_SEC; i > 0 && running; i--)
                  {
                        printf("[%d] ", i);
                        fflush(stdout);
                        SLEEP_MS(1000);
                  }

                  if (!running)
                  {
                        printf("\n[*] Shutdown aborted by user.\n");
                        running = 1; // Reset flag to continue monitoring
                        continue;
                  }

                  printf("\n[*] Executing shutdown...\n");
                  if (execute_shutdown() == 0)
                  {
                        break; // Exit after successful shutdown initiation
                  }
                  else
                  {
                        fprintf(stderr, "[!] Shutdown failed. Continuing to monitor...\n");
                  }
            }
            else if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0)
            {
                  printf("[*] Exit command received. Terminating monitor.\n");
                  break;
            }
            else
            {
                  printf("[*] Command not recognized. Waiting for 'shutdown system'...\n");
            }

            printf("\n");
      }
}

int main(void)
{
      // Register signal handler for graceful shutdown
      signal(SIGINT, handle_interrupt);

      // Start the monitoring loop
      start_text_monitor();

      printf("\n[*] AeroSleep monitor terminated.\n");
      return 0;
}
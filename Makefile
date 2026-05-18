# AeroSleep Makefile
# Cross-platform build configuration

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
TARGET = aerosleep

# Platform detection
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    PLATFORM = Windows
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PLATFORM = Linux
    endif
    ifeq ($(UNAME_S),Darwin)
        PLATFORM = macOS
    endif
endif

.PHONY: all clean run debug

all: $(TARGET)
	@echo "Build complete for $(PLATFORM)"
	@echo "Run with: ./$(TARGET)"

$(TARGET): aerosleep.c
	$(CC) $(CFLAGS) -o $(TARGET) aerosleep.c

debug: CFLAGS += -g -DDEBUG
debug: clean $(TARGET)
	@echo "Debug build complete"

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o *.exe
	@echo "Clean complete"
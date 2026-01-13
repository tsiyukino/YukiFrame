# Simple Makefile for Yuki-Frame v2.0
# For modern builds, use CMake instead

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./include
LDFLAGS = -lpthread

# Detect platform
ifeq ($(OS),Windows_NT)
    PLATFORM = PLATFORM_WINDOWS
    LDFLAGS += -lws2_32
else
    PLATFORM = PLATFORM_LINUX
    CFLAGS += -D_POSIX_C_SOURCE=200809L
    LDFLAGS += -lrt
endif

CFLAGS += -D$(PLATFORM)

TARGET = yuki-frame
SOURCES = src/core/main.c src/core/logger.c src/core/event.c src/core/tool.c \
          src/core/config.c src/core/control.c src/core/debug.c

ifeq ($(PLATFORM),PLATFORM_WINDOWS)
    SOURCES += src/core/platform_windows.c
else
    SOURCES += src/core/platform_linux.c
endif

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

install:
	@echo "Use CMake for installation: cmake --install build"

.PHONY: all clean install

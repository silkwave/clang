# Compiler and flags
CC = cc
CFLAGS = -g -Wall -Wextra -pedantic
LDFLAGS = -lpthread

# Target executable
TARGET = substr_test.exe

# Source and object files
SOURCES = main.c substr.c resource_manager.c
OBJECTS = $(SOURCES:.c=.o)

# Default rule: build the target
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Rule to compile a .c source file into a .o object file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean

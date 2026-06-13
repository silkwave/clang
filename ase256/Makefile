# Compiler and Flags
CC = gcc
# -fsanitize=address: 메모리 오류 탐지 (ASan)
# -g: 디버깅 정보 포함 (오류 발생 시 소스 라인 표시)
CFLAGS = -Wall -Wextra -O2 -g -fsanitize=address -fstack-protector-strong -D_FORTIFY_SOURCE=2
LDFLAGS = -fsanitize=address

# Target names
TARGET = test_ase256
SRC = test_ase256.c
LIB_SRC = ase256.c

# Default rule
all: $(TARGET)

# Compile the test program
$(TARGET): $(SRC) $(LIB_SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

# Run tests
test: $(TARGET)
	./$(TARGET)

# Alias for test
run: test

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all test run clean

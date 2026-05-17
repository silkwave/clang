.POSIX:

CC ?= cc
CFLAGS ?= -std=c11 -O2 -g -Wall -Wextra -Wpedantic -DDEBUG=1
LDFLAGS ?=
LDLIBS ?=

TARGET := bbnk
SRC := bbnk.c
OBJ := $(SRC:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJ)

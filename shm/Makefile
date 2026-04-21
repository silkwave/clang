CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2
LDFLAGS ?=

TARGET := main
SRCS := main.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o

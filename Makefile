CC = gcc
CFLAGS = -Wall -O2
TARGET = utf8test
OBJS = main.o substr_utf8.o resource_manager.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

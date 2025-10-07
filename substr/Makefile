CC = gcc
CFLAGS = -Wall -O2
OBJS = main.o substr.o resource_manager.o
TARGET = utf8test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

CC ?= gcc

SRCS := main.c
BIN := main

CPPFLAGS ?=
CFLAGS ?=
LDFLAGS ?=
LDLIBS ?=

MODE ?= release

CFLAGS += -std=c11 -Wall -Wextra
ifeq ($(MODE),debug)
  CFLAGS += -O0 -g3
else
  CFLAGS += -O2 -DNDEBUG
endif

BUILD_DIR := build/$(MODE)
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all run clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c -o $@ $<

-include $(DEPS)

run: $(BIN)
	./$(BIN) $(ARGS)

clean:
	rm -f $(BIN)
	rm -rf build

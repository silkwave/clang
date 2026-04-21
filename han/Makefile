CC ?= cc
AR ?= ar

CFLAGS ?= -O2 -Wall -Wextra
ARFLAGS ?= rcs

LIB := libhan.a
EXE := example

.PHONY: all lib example clean run

all: lib example

lib: $(LIB)

$(LIB): main.o
	$(AR) $(ARFLAGS) $@ $^

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

example: main.c
	$(CC) $(CFLAGS) -DLIBCMN_EXAMPLE -o $(EXE) $<

run: example
	./$(EXE)

clean:
	rm -f *.o $(LIB) $(EXE)


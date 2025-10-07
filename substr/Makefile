# 이 Makefile은 substr 예제 빌드를 위한 설정입니다.
# - main, substr, resource_manager 소스 파일을 컴파일하여 `utf8test` 실행 파일을 생성합니다.
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

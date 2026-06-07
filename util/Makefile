# 컴파일러 및 옵션 설정
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c99
TARGET  = trim_test

# 소스 파일 목록
SRCS    = main.c 
OBJS    = $(SRCS:.c=.o)

# 1. 기본 빌드 규칙
all: $(TARGET)

# 2. 실행 규칙 (추가됨)
# $(TARGET)에 의존성을 걸어두어, 코드가 수정되면 자동으로 빌드 후 실행합니다.
run: $(TARGET)
	@echo "--- 프로그램 실행 ---"
	./$(TARGET)

# 3. 실행 파일 생성 규칙
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 4. 개별 소스 파일 컴파일 규칙
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 5. 빌드 부산물 정리
clean:
	rm -f $(OBJS) $(TARGET)

# 의사 타겟(Phony Targets) 정의 (run 추가)
.PHONY: all run clean
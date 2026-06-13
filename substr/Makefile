# 컴파일러 및 플래그 설정
CC = cc
CFLAGS = -g -Wall -Wextra -pedantic -std=c11
# 메모리 검증 시 아래 플래그 활성화:
# CFLAGS += -fsanitize=address
# LDFLAGS += -fsanitize=address
LDFLAGS = -lpthread

# 타겟 실행 파일 이름
TARGET = substr_test.exe

# 소스 파일 및 오브젝트 파일 목록
SOURCES = main.c substr.c resource_manager.c
OBJECTS = $(SOURCES:.c=.o)
DEPS = $(SOURCES:.c=.d)

# 기본 빌드 규칙
all: $(TARGET)

# 실행 파일 링크
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# 개별 소스 파일 컴파일 (의존성 파일 생성 포함)
%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# 빌드 결과물 삭제
clean:
	rm -f $(OBJECTS) $(DEPS) $(TARGET)

# 프로그램 실행 규칙
run: all
	./$(TARGET)

# 생성된 의존성 파일 포함 (.h 파일 수정 시 재빌드 보장)
-include $(DEPS)

.PHONY: all clean run

# 컴파일러 설정
CC ?= gcc

# 소스 파일 및 결과물 이름
SRCS := main.c
BIN := main

# 전처리기 및 컴파일 옵션
CPPFLAGS ?=
CFLAGS ?=
LDFLAGS ?=
LDLIBS ?=

# 빌드 모드 (release 또는 debug)
MODE ?= release

# 기본 컴파일 옵션 설정 (C11 표준 사용)
CFLAGS += -std=c11 -Wall -Wextra
ifeq ($(MODE),debug)
  # 디버그 모드: 최적화 해제 및 디버깅 정보 포함
  CFLAGS += -O0 -g3
else
  # 릴리즈 모드: 최적화 활성화 및 NDEBUG 정의
  CFLAGS += -O2 -DNDEBUG
endif

# 빌드 결과물 저장 경로 설정
BUILD_DIR := build/$(MODE)
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# 가상 타겟 정의
.PHONY: all run clean

# 기본 타겟: 프로그램 빌드
all: $(BIN)

# 실행 파일 생성 규칙
$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# 빌드 디렉토리 생성 규칙
$(BUILD_DIR):
	mkdir -p $@

# 오브젝트 파일 생성 규칙 (의존성 파일 생성 포함)
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c -o $@ $<

# 생성된 의존성 파일 포함
-include $(DEPS)

# 프로그램 실행 타겟
run: $(BIN)
	./$(BIN) $(ARGS)

# 빌드 결과물 삭제 타겟
clean:
	rm -f $(BIN)
	rm -rf build

# Makefile for base26.c
# Target : 10진수 <-> 26진수(0-9, A-P) 변환 유틸리티
# Compiler: clang이 설치되어 있으면 자동 사용, 없으면 gcc로 자동 폴백
#
# 사용법:
#   make            # 릴리즈 빌드 (최적화 O2)
#   make debug      # 디버그 빌드 (-g, ASan/UBSan 포함)
#   make run        # 빌드 후 즉시 실행
#   make clean      # 빌드 산출물 정리
#   make CC=gcc     # 컴파일러를 수동으로 gcc로 고정
#   make CC=clang   # 컴파일러를 수동으로 clang으로 고정

# clang 설치 여부를 자동 탐지하여 없으면 gcc로 폴백.
# 셸에 CC가 export되어 있으면 그 값을 그대로 존중함 (make 기본 동작).
ifeq ($(origin CC),default)
  CC := $(shell command -v clang >/dev/null 2>&1 && echo clang || echo gcc)
endif

TARGET  := base26
SRC     := base26.c
BUILDIR := build

# override: 셸 환경변수(CSTD 등)가 잘못 섞여 들어와도 항상 아래 값을 강제 사용
override CSTD     := -std=c11
override WARNINGS := -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
override RELEASE_FLAGS := -O2 -DNDEBUG
override DEBUG_FLAGS   := -O0 -g3 -fsanitize=address,undefined -fno-omit-frame-pointer

CFLAGS  := $(CSTD) $(WARNINGS) $(RELEASE_FLAGS)

.PHONY: all debug run clean

all: $(BUILDIR)/$(TARGET)

$(BUILDIR)/$(TARGET): $(SRC) | $(BUILDIR)
	$(CC) $(CFLAGS) -o $@ $(SRC)

$(BUILDIR):
	mkdir -p $(BUILDIR)

debug: CFLAGS := $(CSTD) $(WARNINGS) $(DEBUG_FLAGS)
debug: $(BUILDIR)/$(TARGET)

run: all
	./$(BUILDIR)/$(TARGET)

clean:
	rm -rf $(BUILDIR)
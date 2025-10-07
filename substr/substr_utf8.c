#include "substr_utf8.h"
#include "resource_manager.h"
#include <stdlib.h>
#include <string.h>

/// UTF-8 문자 1개가 차지하는 바이트 수 계산
/// (첫 바이트 값을 통해 길이를 판별)
static int utf8_char_len(unsigned char c) {
    if (c < 0x80) return 1;          // ASCII (1바이트)
    else if ((c >> 5) == 0x6) return 2;  // 110xxxxx (2바이트)
    else if ((c >> 4) == 0xE) return 3;  // 1110xxxx (3바이트)
    else if ((c >> 3) == 0x1E) return 4; // 11110xxx (4바이트)
    return 1;  // 잘못된 경우 기본 1바이트 처리
}

/// UTF-8 문자열에서 SQL의 SUBSTR과 동일하게 동작하는 함수
///
/// @param str     원본 문자열
/// @param start   시작 인덱스 (1부터 시작, 음수면 뒤에서부터)
/// @param length  잘라낼 문자 수
/// @return        새로 할당된 문자열 포인터 (프레임워크가 자동 해제)
///
/// @note malloc()으로 메모리를 할당하며,
///       `register_resource(result)`를 통해 프레임워크에 자동 관리 등록.
char* substr_utf8(const char* str, int start, int length) {
    if (!str) return NULL;

    // 1️⃣ 총 문자 수 계산
    int total_chars = 0;
    const char* p = str;
    while (*p) {
        total_chars++;
        p += utf8_char_len((unsigned char)*p);
    }

    // 2️⃣ SQL 스타일 인덱스 보정
    // start: 1-based → 0-based 변환
    if (start > 0) start -= 1;
    else if (start < 0) start = total_chars + start;

    if (start < 0 || start >= total_chars) return NULL;
    if (length <= 0 || start + length > total_chars)
        length = total_chars - start;

    // 3️⃣ 시작 바이트 위치로 이동
    p = str;
    int idx = 0;
    while (idx < start) {
        p += utf8_char_len((unsigned char)*p);
        idx++;
    }

    // 4️⃣ 잘라낼 끝 위치 계산
    const char* q = p;
    idx = 0;
    while (idx < length && *q) {
        q += utf8_char_len((unsigned char)*q);
        idx++;
    }

    // 5️⃣ 새 문자열 메모리 할당
    int byte_len = q - p;
    char* result = malloc(byte_len + 1);
    if (!result) return NULL;

    // 6️⃣ 문자열 복사 및 종료 문자 추가
    memcpy(result, p, byte_len);
    result[byte_len] = '\0';

    // 7️⃣ 프레임워크 자원관리 목록에 등록 (종료 시 자동 free)
    register_resource(result);

    return result;
}

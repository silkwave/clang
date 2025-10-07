/*
 * substr.c
 * - 멀티바이트(UTF-8, MS949) 문자열에서 문자 단위로 부분 문자열을 추출하는 구현
 *
 * 내부 동작 요약:
 * 1) 입력 문자열의 전체 문자 수를 계산한다.
 * 2) 시작 위치와 길이를 문자 단위로 조정한다.
 * 3) 해당 문자 범위에 해당하는 바이트 구간을 복사하여 새 문자열을 할당하고 반환한다.
 *
 * 주의: 반환된 메모리는 `register_resource`로 등록되어 자동으로 추적/해제됩니다.
 */

#include "substr.h"
#include "resource_manager.h"
#include <stdlib.h>
#include <string.h>

/*
 * char_len: 포인터가 가리키는 문자에서 그 문자의 바이트 길이를 반환
 * - UTF-8: 선행 바이트의 비트를 검사하여 1~4바이트를 판정
 * - MS949: ASCII(0x00-0x7F)는 1바이트, 그 외는 2바이트로 간주
 */
static int char_len(const char* p, encoding_t enc) {
    unsigned char c = (unsigned char)*p;
    if (enc == ENCODING_UTF8) {
        if (c < 0x80) return 1;
        else if ((c >> 5) == 0x6) return 2;
        else if ((c >> 4) == 0xE) return 3;
        else if ((c >> 3) == 0x1E) return 4;
        return 1;
    } else { // MS949
        return (c < 0x80) ? 1 : 2;
    }
}

char* substr(const char* str, int start, int length, encoding_t enc) {
    if (!str) return NULL;

    int total_chars = 0;
    const char* p = str;
    while (*p) {
        total_chars++;
        p += char_len(p, enc);
    }

    /*
     * 사용자 친화적 인덱스 처리
     * - start > 0 : 1 기반 인덱스(사용자 입력 가정)을 0 기반으로 변환
     * - start < 0 : 끝에서부터의 오프셋 지원
     */
    if (start > 0) start -= 1;
    else if (start < 0) start = total_chars + start;

    if (start < 0 || start >= total_chars) return NULL;
    if (length <= 0 || start + length > total_chars)
        length = total_chars - start;

    p = str;
    int idx = 0;
    while (idx < start) {
        p += char_len(p, enc);
        idx++;
    }

    const char* q = p;
    idx = 0;
    while (idx < length && *q) {
        q += char_len(q, enc);
        idx++;
    }

    int byte_len = q - p;
    char* result = malloc(byte_len + 1);
    if (!result) return NULL;

    memcpy(result, p, byte_len);
    result[byte_len] = '\0';

    /* 할당된 문자열을 리소스 매니저에 등록하여 프로그램 종료 시 자동 해제 */
    register_resource(result);

    return result;
}

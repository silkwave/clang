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
static int char_len(const char *p, encoding_t enc)
{
    unsigned char c = (unsigned char)*p;
    if (enc == ENCODING_UTF8)
    {
        if (c < 0x80) // 0xxxxxxx
            return 1;
        // 110xxxxx 10xxxxxx
        else if ((c & 0xE0) == 0xC0 && (p[1] & 0xC0) == 0x80)
            return 2;
        // 1110xxxx 10xxxxxx 10xxxxxx
        else if ((c & 0xF0) == 0xE0 && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80)
            return 3;
        // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        else if ((c & 0xF8) == 0xF0 && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80 && (p[3] & 0xC0) == 0x80)
            return 4;

        // 잘못된 바이트 시퀀스일 경우 1바이트 전진
        return 1;
    }
    else
    { // MS949
        return (c < 0x80) ? 1 : 2;
    }
}

char *substr(const char *str, int start, int length, encoding_t enc)
{
    if (!str)
        return NULL; // 입력 문자열이 NULL이면 NULL 반환

    if (length < 0)  // 길이가 음수이면 처리 불가
        return NULL; // 추출할 길이가 음수이면 처리 불가, NULL 반환

    const char *p = str;
    const char *start_ptr = NULL;
    const char *end_ptr = NULL;
    int current_char_index = 0;

    // 1-based to 0-based index
    // 사용자가 입력한 1-기반 인덱스를 0-기반으로 변환
    if (start > 0)
    {
        start--;
    }
    else if (start < 0) // 음수 인덱스 처리 (문자열 끝에서부터의 위치)
    {
        // 음수 인덱스를 계산하려면 먼저 전체 문자 수를 세어야 함
        int total_chars = 0;
        const char *temp = str;
        while (*temp)
        {
            total_chars++;
            temp += char_len(temp, enc);
        }
        start = total_chars + start; // 끝에서부터의 위치를 0-기반 인덱스로 변환
    }

    if (start < 0)
        return NULL; // 인덱스 조정 후에도 음수이면 범위를 벗어난 것이므로 NULL 반환

    // 문자열을 한 번만 순회하여 시작 포인터와 끝 포인터를 찾음
    while (*p)
    {
        if (current_char_index == start)
        {
            start_ptr = p;
        }
        if (length > 0 && current_char_index == start + length)
        {
            end_ptr = p; // 끝 위치의 포인터 저장
        }
        p += char_len(p, enc);
        current_char_index++;
    }

    if (!start_ptr)
        return NULL; // 시작 포인터를 찾지 못했다면 시작 인덱스가 범위를 벗어난 것

    // 끝 포인터를 찾지 못한 경우 처리
    if (!end_ptr)
    {
        if (length == 0)
        {
            end_ptr = start_ptr; // 길이가 0이면 시작과 끝 포인터가 같음 (빈 문자열)
        }
        else
        {
            // 길이가 문자열 끝을 넘어가면, 문자열의 끝(\0)을 끝 포인터로 설정
            end_ptr = p;
        }
    }

    // 시작과 끝 포인터의 차이로 필요한 바이트 길이 계산
    int byte_len = end_ptr - start_ptr;
    char *result = malloc(byte_len + 1); // 바이트 길이 + 널 종료 문자를 위한 공간 할당
    if (!result)
        return NULL;

    memcpy(result, start_ptr, byte_len); // 계산된 바이트만큼 메모리 복사
    result[byte_len] = '\0';             // 문자열의 끝에 널 종료 문자 추가

    /* 할당된 문자열을 리소스 매니저에 등록하여 프로그램 종료 시 자동 해제 */
    register_resource(result);

    return result;
}

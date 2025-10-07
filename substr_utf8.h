#ifndef SUBSTR_UTF8_H
#define SUBSTR_UTF8_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * UTF-8 문자열에서 SQL SUBSTR과 동일한 동작
 * @param str 원본 문자열
 * @param start 시작 인덱스 (1부터 시작, 음수는 뒤에서부터)
 * @param length 잘라낼 문자 수 (0 또는 음수면 끝까지)
 * @return malloc된 문자열, 자동 등록됨 → free() 필요 없음
 */
char* substr_utf8(const char* str, int start, int length);

#ifdef __cplusplus
}
#endif

#endif // SUBSTR_UTF8_H

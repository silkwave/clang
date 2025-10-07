/*
 * substr.h
 * - 다국어(UTF-8, MS949) 문자열에서 문자 단위로 부분 문자열을 추출하는 인터페이스
 *
 * 함수: substr
 * 입력: const char* str - 원본 문자열
 *       int start - 시작 문자 인덱스 (1부터 시작, 음수는 끝에서부터)
 *       int length - 추출할 문자 수
 *       encoding_t enc - 문자열 인코딩 (UTF-8 또는 MS949)
 * 출력: 동적 할당된 C 문자열 (사용 후 해제 또는 리소스 매니저를 통해 관리)
 */

#ifndef SUBSTR_H
#define SUBSTR_H

typedef enum {
    ENCODING_UTF8,
    ENCODING_MS949
} encoding_t;

/*
 * substr: 문자열에서 지정된 문자 범위를 바이트 단위로 잘라 새 문자열로 반환합니다.
 * 반환된 메모리는 내부적으로 `register_resource`로 등록되어 프로그램 종료 시 자동 해제됩니다.
 */
char* substr(const char* str, int start, int length, encoding_t enc);

#endif // SUBSTR_H

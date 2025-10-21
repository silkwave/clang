/*
 * main.c
 * 예제 실행 파일
 * - UTF-8 / MS949 가정의 문자열에서 부분 문자열을 추출하여 출력합니다.
 */

#include <stdio.h>
#include "substr.h"
#include "resource_manager.h" // For unregister_resource

/* substr 함수의 다양한 사용 예를 테스트하는 함수 */
static void run_all_tests(void)
{
    const char *utf8_text = "안녕하세요 Hello World!";
    const char *ms949_text = "안녕하세요 Hello World!"; // MS949 인코딩 가정

    printf("--- Basic Tests ---\n");
    // 기본 테스트: UTF-8과 MS949 문자열에서 앞 5글자를 추출합니다.
    char *u1 = substr(utf8_text, 1, 5, ENCODING_UTF8);
    char *m1 = substr(ms949_text, 1, 5, ENCODING_MS949);
    printf("UTF-8 (1, 5): [%s]\n", u1);
    printf("MS949 (1, 5): [%s]\n", m1);

    printf("\n--- Edge Case Tests ---\n");
    // 엣지 케이스 1: 음수 인덱스 테스트 (끝에서부터 6번째 문자부터 5글자)
    char *u2 = substr(utf8_text, -6, 5, ENCODING_UTF8);
    printf("UTF-8 (-6, 5): [%s]\n", u2); // "World"

    // 엣지 케이스 2: 길이가 문자열의 끝을 초과하는 경우
    char *u3 = substr(utf8_text, 7, 100, ENCODING_UTF8);
    printf("UTF-8 (7, 100): [%s]\n", u3); // "Hello World!"

    // 엣지 케이스 3: 길이가 0인 경우 (빈 문자열이 반환되어야 함)
    char *u4 = substr(utf8_text, 1, 0, ENCODING_UTF8);
    printf("UTF-8 (1, 0): [%s]\n", u4 ? u4 : "(null)"); // Should be empty or handled

    // 엣지 케이스 4: 시작 인덱스가 범위를 벗어난 경우 (NULL이 반환되어야 함)
    char *u5 = substr(utf8_text, 100, 5, ENCODING_UTF8);
    printf("UTF-8 (100, 5): [%s]\n", u5 ? u5 : "(null)");

    // 리소스 관리자 테스트: 수동으로 리소스 하나를 해제합니다.
    printf("\nManually unregistering 'u1'...\n");
    unregister_resource(u1);
    u1 = NULL; // 이중 해제를 방지하기 위해 포인터를 NULL로 설정합니다.

    // 테스트에 사용된 모든 리소스를 정리합니다.
    printf("\n--- Cleaning up test resources ---\n");
    cleanup_resources();
}

/* 프로그램의 메인 진입점 */
int main(void)
{
    run_all_tests(); // 모든 테스트를 실행합니다.
    return 0;
}

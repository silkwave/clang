#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <stddef.h>

// 26진수 변환에 사용할 기수와 문자 배열 (0~9, A~P)
enum { BASE26 = 26 };
static const char DIGITS[] = "0123456789ABCDEFGHIJKLMNOP";

// 26진수 한 문자의 값을 반환 (유효하지 않으면 -1)
static int digit_value(char chr) {
    if (chr >= '0' && chr <= '9') return chr - '0';
    if (chr >= 'A' && chr <= 'P') return chr - 'A' + 10;
    return -1;
}

// 10진수 숫자를 26진수 문자열로 변환하는 함수
bool dec26(long num, char *buf, size_t siz) {
    // 유효성 검사: 버퍼 유효성, 최소 크기(NULL 포함 2자 이상), 양수 여부
    if (!buf || siz < 2 || num < 0) return false;

    // 변환 후 문자열의 필요한 자릿수 계산 (0은 1자리)
    size_t len = 1;
    for (long tmp = num; tmp >= BASE26; tmp /= BASE26) len++;
    if (len >= siz) return false; // 버퍼 크기 초과 시 실패

    buf[len] = '\0'; // 문자열 종단 문자 설정
    // 숫자를 26으로 나누어가며 뒤쪽에서부터 문자를 채움
    do {
        buf[--len] = DIGITS[num % BASE26];
        num /= BASE26;
    } while (num);

    return true;
}

// 26진수 문자열을 10진수 숫자로 변환하는 함수
bool todec(const char *str, long *num) {
    // 유효성 검사: 포인터 및 빈 문자열 여부
    if (!str || !num || !*str) return false;

    long res = 0;
    for (; *str; str++) {
        int val = digit_value(*str);

        // 유효하지 않은 문자이거나 LONG_MAX 오버플로우 발생 시 실패 처리
        if (val < 0 || res > (LONG_MAX - val) / BASE26) return false;
        res = res * BASE26 + val; // 26진수 가중치 적용 및 더하기
    }

    *num = res; // 변환 결과 저장
    return true;
}

// dec26 결과를 출력하는 테스트 헬퍼
static void test_dec26(long num) {
    char buf[32];
    if (dec26(num, buf, sizeof buf)) printf("dec26(%ld) = %s\n", num, buf);
    else                             printf("dec26(%ld) = <실패>\n", num);
}

// todec 결과를 출력하는 테스트 헬퍼
static void test_todec(const char *str) {
    long num = 0;
    if (todec(str, &num)) printf("todec(\"%s\") = %ld\n", str, num);
    else                  printf("todec(\"%s\") = <실패>\n", str);
}

int main(void) {
    test_dec26(0);
    test_dec26(25);
    test_dec26(456975);     // 26^4 - 1 -> "PPPP"
    test_dec26(123456789L);
    test_dec26(-1);         // 음수 -> 실패

    test_todec("0");
    test_todec("PPPP");
    test_todec("123456789");
    test_todec("Z");        // 잘못된 문자 -> 실패
    test_todec("");         // 빈 문자열 -> 실패

    return 0;
}

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <stddef.h>

// 26진수 변환에 사용할 문자 배열 (0~9, A~P)
static const char CHS[] = "0123456789ABCDEFGHIJKLMNOP";

// 10진수 숫자를 26진수 문자열로 변환하는 함수
bool dec26(long num, char *buf, size_t siz) {
    // 유효성 검사: 버퍼 유효성, 최소 크기(NULL 포함 2자 이상), 양수 여부
    if (!buf || siz < 2 || num < 0) return false;

    // 변환 후 문자열의 필요한 자릿수 계산
    size_t len = 0;
    for (long tmp = num; len == 0 || tmp; tmp /= 26) len++;
    if (len >= siz) return false; // 버퍼 크기 초과 시 실패

    buf[len] = '\0'; // 문자열 종단 문자 설정
    // 숫자를 26으로 나누어가며 뒤쪽에서부터 문자를 채움
    do {
        buf[--len] = CHS[num % 26];
        num /= 26;
    } while (num);

    return true;
}

// 26진수 문자열을 10진수 숫자로 변환하는 함수
bool todec(const char *str, long *num) {
    // 유효성 검사: 포인터 및 빈 문자열 여부
    if (!str || !num || !*str) return false;

    long res = 0;
    for (; *str; str++) {
        char chr = *str;
        // 각 문자를 해당하는 10진 값으로 파싱 (유효하지 않으면 -1)
        int val = (chr >= '0' && chr <= '9') ? (chr - '0') :
                  (chr >= 'A' && chr <= 'P') ? (chr - 'A' + 10) : -1;

        // 유효하지 않은 문자이거나 LONG_MAX 오버플로우 발생 시 실패 처리
        if (val < 0 || res > (LONG_MAX - val) / 26) return false;
        res = res * 26 + val; // 26진수 가중치 적용 및 더하기
    }

    *num = res; // 변환 결과 저장
    return true;
}

int main(void) {
    long num1 = 123456789L, num2 = 0;
    char buf[32];

    // 변환 테스트 실행 및 출력
    if (dec26(num1, buf, sizeof(buf))) printf("26진수 : %s\n", buf);
    if (todec(buf, &num2))             printf("10진수 : %ld\n", num2);

    if (todec("PPPPP", &num2))          printf("10진수 : %ld\n", num2); 
   
    return 0;
}
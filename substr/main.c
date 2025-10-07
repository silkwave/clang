#include <stdio.h>
#include "substr.h"

int main(void) {
    const char* utf8_text = "안녕하세요 Hello World!";
    const char* ms949_text = "안녕하세요 Hello World!"; // MS949 인코딩 가정

    char* u1 = substr(utf8_text, 1, 5, ENCODING_UTF8);
    char* m1 = substr(ms949_text, 1, 5, ENCODING_MS949);

    printf("UTF-8: [%s]\n", u1);
    printf("MS949: [%s]\n", m1);

    return 0;
}

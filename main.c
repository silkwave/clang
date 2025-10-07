#include <stdio.h>
#include <string.h>
#include "resource_manager.h"
#include "substr_utf8.h"

int main(void) {
    const char* text = "안녕하세요 세상! Hello World!";

    char* s1 = substr_utf8(text, 1, 2);   // "안녕"
    char* s2 = substr_utf8(text, -6, 5);  // "Hello"

    printf("[등록 후 개수] %d\n", resource_count());

    // realloc 테스트
    s2 = realloc_resource(s2, 64);
    strcat(s2, "!!!");
    printf("변경된 s2: %s\n", s2);

    // unregister 테스트
    unregister_resource(s1);
    printf("[s1 해제 후 개수] %d\n", resource_count());

    return 0;
}

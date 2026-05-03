#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * 1. Java의 String.contains() 기능 구현 (부분 문자열 검색)
 * @param haystack 검색 대상이 되는 전체 문자열
 * @param needle 찾고자 하는 부분 문자열
 * @return 포함되어 있으면 true, 아니면 false
 */
bool contains(const char *haystack, const char *needle) {
    if (haystack == NULL || needle == NULL) return false;
    // 빈 문자열은 항상 포함된 것으로 간주 (Java 동작 방식)
    if (*needle == '\0') return true;
    
    return strstr(haystack, needle) != NULL;
}

/**
 * 2. Java의 List.contains() 기능 구현 (문자열 배열 내 요소 검색)
 * @param list NULL로 끝나는 문자열 배열
 * @param target 찾고자 하는 정확한 문자열
 * @return 배열 내에 존재하면 true, 아니면 false
 */
bool list_contains(const char *list[], const char *target) {
    if (list == NULL || target == NULL) return false;
    
    // 포인터를 사용하여 배열의 끝(NULL)까지 순회
    for (const char **p = list; *p != NULL; p++) {
        if (strcmp(*p, target) == 0) {
            return true;
        }
    }
    return false;
}

int main() {
    // 차단된 MCN 목록 (마지막에 NULL을 추가하여 크기 관리 없이 순회 가능)
    const char *blocked_mcns[] = {
        "051", "052", "053", "054", "055", 
        "056", "057", "058", "059", NULL
    };

    printf("=== Java 스타일 문자열 유틸리티 테스트 ===\n\n");

    // 예제 1: 배열 내 특정 요소 존재 확인 (list_contains)
    const char *target = "059";
    printf("[배열 검색] '%s'이(가) 차단 목록에 있는가?\n", target);
    if (list_contains(blocked_mcns, target)) {
        printf(" -> 결과: YES, 차단된 MCN입니다.\n\n");
    } else {
        printf(" -> 결과: NO, 허용된 MCN입니다.\n\n");
    }

    // 예제 2: 문자열 내 부분 문자열 포함 확인 (contains)
    const char *raw_data = "DEVICE_ID_054_SEOUL";
    const char *check_val = "054";
    printf("[부분 검색] '%s' 내에 '%s'이(가) 포함되어 있는가?\n", raw_data, check_val);
    if (contains(raw_data, check_val)) {
        printf(" -> 결과: YES, 포함되어 있습니다.\n");
    } else {
        printf(" -> 결과: NO, 포함되어 있지 않습니다.\n");
    }

    return 0;
}

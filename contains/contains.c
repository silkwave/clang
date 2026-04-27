#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * [함수] contains
 * 특정 문자열 배열 안에 찾고자 하는 대상 문자열이 존재하는지 검사합니다.
 * 
 * @param arr    : 검색 대상이 되는 문자열 포인터 배열 (가리키는 내용과 포인터 주소 모두 수정 불가)
 * @param size   : 배열에 담긴 요소의 개수
 * @param target : 찾으려는 대상 문자열
 * @return       : 찾았으면 true(참), 못 찾았거나 입력이 잘못되었으면 false(거짓) 반환
 */
bool contains(const char *const *arr, int size, const char *target) {
    // 1. 방어 코드: 인자값이 NULL인 경우 예외 처리
    if (arr == NULL || target == NULL) {
        return false;
    }

    // 2. 배열의 크기만큼 루프를 돌며 하나씩 비교
    for (int i = 0; i < size; i++) {
        // 배열의 각 요소가 NULL이 아닌지 확인 후 문자열 비교 수행
        if (arr[i] != NULL && strcmp(arr[i], target) == 0) {
            return true; // 일치하는 항목을 찾으면 즉시 true 반환 (조기 종료)
        }
    }

    // 3. 루프를 끝까지 돌았음에도 찾지 못한 경우
    return false;
}

/**
 * [메인 함수]
 * 프로그램의 실행 시작점입니다.
 */
int main(int argc, char const *argv[]) {
    // [설정] 차단 대상 기기 번호(MCN) 목록 정의
    const char *blocked_mcns[] = {
        "051", "052", "053", "054", "055", 
        "056", "057", "058", "059"
    };

    // [계산] 배열의 전체 크기를 요소 하나의 크기로 나누어 개수(size) 산출
    int size = sizeof(blocked_mcns) / sizeof(blocked_mcns[0]);

    // [입력] 명령행 인자가 있으면 해당 값을 사용하고, 없으면 기본값 "060" 사용
    // 예: ./contains 051
    const char *target = (argc >= 2) ? argv[1] : "060";

    printf("[로그] 검사 대상 기기 번호(MCN): %s\n", target);

    // [실행] contains 함수를 호출하여 차단 목록에 포함되어 있는지 확인
    if (contains(blocked_mcns, size, target)) {
        // 차단 목록에 존재하는 경우
        fprintf(stderr, "[결과] 에러: [%s]는 차단된 기기 번호입니다.\n", target);
        return -1010; // 업무 규칙에 따른 오류 리턴 코드
    }

    // [종료] 차단되지 않은 경우 정상 메시지 출력 후 종료
    puts("[결과] 정상: 통과되었습니다. (OK)");
    return 0; // 성공 리턴 코드
}

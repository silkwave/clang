#include <string.h>
#include <stdio.h>

// 예시 리턴코드(4바이트) - 실제 프로젝트에서는 공통 헤더/정의값을 사용하세요.
#define CHK_OFC_PS_LMT "LMT1"

// arr(문자열 포인터 배열) 안에 target 문자열이 존재하는지 검사한다.
// - arr: 문자열 포인터 배열(const char*)
// - size: arr 원소 개수
// - target: 비교 대상 문자열
// 반환값: 있으면 1, 없으면 0
int contains(const char *const *arr, int size, const char *target)
{
    for (int idx = 0; idx < size; idx++)
    {
        if (strcmp(arr[idx], target) == 0)
        {
            return 1; // true(포함)
        }
    }
    return 0; // false(미포함)
}

int main(int argc, char const *argv[])
{
    // 입력은 "소스 내 기본값" 또는 "명령행 인자"로 처리한다.
    // - 기본값으로 실행: ./contains
    // - 인자로 실행:     ./contains <OfcCode> <McnNo>
    const char *ofcCode = "001571";
    const char *mcnNo = "060";
    // 4바이트 리턴코드 + '\0'
    char rc_RtCode[5] = {0};

    if (argc >= 2)
    {
        ofcCode = argv[1];
    }
    if (argc >= 3)
    {
        mcnNo = argv[2];
    }

    // 특정 지점코드(예: "001571")에서만 MCN 차단 리스트를 적용한다.
    if (strcmp(ofcCode, "001571") == 0)
    {
        // 차단 대상 MCN 목록(문자열 포인터 배열)
        const char *mcnList[] = {"051", "052", "053", "054", "055", "056", "057", "058", "059"};

        int size = (int)(sizeof(mcnList) / sizeof(mcnList[0]));

        // 목록에 포함되면 리턴코드를 세팅하고 오류로 종료한다.
        if (contains(mcnList, size, mcnNo))
        {
            memcpy(rc_RtCode, CHK_OFC_PS_LMT, 4);
            rc_RtCode[4] = '\0';
            fprintf(stderr, "blocked (rc_RtCode=%s)\n", rc_RtCode);
            return -1010;
        }
    }

    puts("OK");
    return 0;
}

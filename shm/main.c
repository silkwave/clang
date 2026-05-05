/*
 * main.c
 * 공유메모리(SHM) 상태 정보를 가독성 있게 출력하는 유틸리티 예제
 *
 * 이 프로그램은 실제 금융 시스템 등에서 사용되는 업무별 장애 상태 관리용
 * 공유메모리 구조를 로컬 환경에서 시뮬레이션하고 시각화하는 도구입니다.
 *
 * - 주요 구조체: HOST_INFO_SHM (50x50 업무 장애 상태 행렬 포함)
 * - 주요 기능: 매체별 장애 요약 출력, 50x50 매트릭스 시각화, 단위 테스트
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>

/* ----------------------------------------------------------------
 * 공유메모리 구조체 정의 (str_apcmn.h 발췌)
 * ---------------------------------------------------------------- */

#define MAX_HOST_INFO_CNT 2 /* 최대 호스트 정보 개수 */

typedef struct
{
    char JungJoSect[1];        /* 중조구분                        */
    char DealDate[8];          /* 거래일정보                      */
    char WkInx[1];             /* 요일인덱스                      */
    char BankHldYn[1];         /* 금융휴무여부                    */
    char HldYn[1];             /* 휴일여부                        */
    char DateBatchYn[1];       /* 일자batch수행여부               */
    char BankMCAStat[1];       /* 신용MCA상태 1:개국 2:폐국       */
    char ShmLoadYn[1];         /* SharedMemory 전체 항목 load여부 */
    char SvrErrYn[1];          /* AP서버 전체업무 장애            */
    char TaskHaltStat[50][50]; /* 업무별 장애 상태 [매체][업무]   */
    char FlowCtl[10];          /* 유량 제어 상태                  */
    char Filler[100];          /* 향후 확장을 위한 여유 공간      */
} HOST_INFO_SHM;

#define SZ_HOST_INFO_SHM (sizeof(HOST_INFO_SHM))

/* ----------------------------------------------------------------
 * 전역 변수 및 상태 관리
 * ---------------------------------------------------------------- */

/* 공유메모리 인스턴스를 시뮬레이션하기 위한 정적 배열 */
static HOST_INFO_SHM g_host_info_shm[MAX_HOST_INFO_CNT];
static int g_host_info_idx = 0; /* 현재 활성화된 호스트 인덱스 */

/* ----------------------------------------------------------------
 * 매체(Media) 및 업무(Job) 마스터 데이터
 * ---------------------------------------------------------------- */

/* 매체 명칭 테이블 (인덱스 0~49 대응) */
char Media[50][32] = {
    "IS03 M/S 신용겸용", "IS03 M/S 중앙카드", "IS03 M/S 조합카드", "IS03 M/S 가상계좌",
    "IS03 M/S 타행카드", "IC 자행카드", "IC 타행카드", "RF 자행모바일",
    "RF 타행모바일", "IRFM 모바일", "통장 요구불", "통장 신탁",
    "통장 저축성", "통장 수익증권", "통장 공제", "전자통장 요구불",
    "전자통장 신탁", "전자통장 저축성", "전자통장 수익증권", "전자화폐 자행",
    "전자화폐 타행", "무통장 요구불", "무통장 신탁", "무통장 저축성",
    "무통장 수익증권", "IS02 M/S 자행", "IS02 M/S 타행", "IS02 M/S 타사",
    "IS02 M/S 해외", "IS02 EMV 자행", "IS02 EMV 타행", "IS02 EMV 타사",
    "(미정)", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)",
    "(미정)", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)",
    "(미정)", "(미정)"
};

/* 업무 명칭 테이블 (인덱스 0~49 대응) */
char Job[50][32] = {
    "잔액조회", "내역조회", "대금조회", "현금지급",
    "수표지급", "현금입금", "수표입금", "CMS집금",
    "계좌이체", "CMS이체", "카드론이체", "입금이체",
    "출금이체", "동행이체", "삼행이체", "통장정리",
    "아파트관리비", "공과금수납", "사고신고", "비밀번호변경",
    "전자화폐충전환불", "신용선결제", "신용청구대금결제", "대학등록금",
    "공제납부", "여신상환", "지방세", "전자납부",
    "2D바코드", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)",
    "(미정)", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)",
    "(미정)", "(미정)", "(미정)", "(미정)", "(미정)", "(미정)"
};

/* ----------------------------------------------------------------
 * 유틸리티 함수
 * ---------------------------------------------------------------- */

/**
 * 색상 텍스트 출력을 위한 래퍼 함수 (ANSI Escape Code 사용)
 * @param color 색상 이름 (red, yellow, sky, green, white)
 * @param fmt   printf 형식 문자열
 */
void cprintf(const char *color, const char *fmt, ...)
{
    va_list ap;
    const char *code = "\033[0m";

    if (strcmp(color, "red") == 0)
        code = "\033[0;31m";
    else if (strcmp(color, "yellow") == 0)
        code = "\033[0;33m";
    else if (strcmp(color, "sky") == 0)
        code = "\033[0;36m";
    else if (strcmp(color, "green") == 0)
        code = "\033[0;32m";
    else if (strcmp(color, "white") == 0)
        code = "\033[0;37m";

    printf("%s", code);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\033[0m");
}

/**
 * 버퍼 내에 유의미한 데이터('1' 등)가 있는지 검사하는 함수
 * @param pc_Src 검사할 데이터 주소
 * @param pi_Len 검사할 길이
 * @return 유의미한 값이 있으면 1, 없으면 0
 */
int sub_CheckValue(char *pc_Src, int pi_Len)
{
    int i;
    for (i = 0; i < pi_Len; i++)
    {
        /* 0x00(NULL), 0x20(공백), 0x30('0')이 아닌 값이 있으면 데이터가 있는 것으로 간주 */
        if (pc_Src[i] != 0x00 && pc_Src[i] != 0x20 && pc_Src[i] != 0x30)
            return 1;
    }
    return 0;
}

/**
 * 프로세스 종료 시 호출되는 시그널 핸들러
 */
void do_exit(int sig)
{
    printf("\n[정보] 프로세스 종료 중... (시그널: %d)\n", sig);
    exit(0);
}

/* ----------------------------------------------------------------
 * 공유메모리 데이터 조작 및 출력 함수
 * ---------------------------------------------------------------- */

/**
 * 특정 매체/업무의 장애 상태를 설정하는 함수
 * @param media_idx 매체 인덱스 (0~49)
 * @param job_idx   업무 인덱스 (0~49)
 * @param value     설정할 값 ('1':장애, '0':정상)
 * @return 성공 시 0, 실패 시 -1
 */
int update_task_halt_stat(int media_idx, int job_idx, char value)
{
    if (media_idx < 0 || media_idx >= 50 || job_idx < 0 || job_idx >= 50)
        return -1;

    g_host_info_shm[g_host_info_idx].TaskHaltStat[media_idx][job_idx] = value;
    return 0;
}

/**
 * 50x50 장애 상태 전체 매트릭스를 화면에 출력
 */
void print_task_halt_stat_all(void)
{
    int i, j;

    cprintf("sky", "\n<< TaskHaltStat 전체 행렬 (0:정상, 1:장애) >>\n");

    /* 상단 인덱스 출력 */
    cprintf("white", "      ");
    for (j = 0; j < 50; j++)
        printf("%d", (j / 10) % 10);
    printf("\n      ");
    for (j = 0; j < 50; j++)
        printf("%d", j % 10);
    printf("\n");

    for (i = 0; i < 50; i++)
    {
        cprintf("white", "[%02d] ", i);
        printf("%-20s ", (Media[i][0] != '\0') ? Media[i] : "(미정)");

        for (j = 0; j < 50; j++)
        {
            unsigned char v = (unsigned char)g_host_info_shm[g_host_info_idx].TaskHaltStat[i][j];
            if (v == '1')
                cprintf("red", "1");
            else
                printf("0");
        }
        printf("\n");
    }
}

/**
 * 공유메모리 전체 상태를 종합적으로 시각화하여 출력
 */
void shm_all_view(void)
{
    int i, j, prnt_cnt;

    cprintf("green", "\n*** 공유메모리(SHM) 상태 조회 결과 ***\n");

    /* 매체별 업무장애 요약 출력 */
    cprintf("sky", "\n<< 장애 발생 매체 및 업무 요약 >>");
    for (i = 0; i < 50; i++)
    {
        if (Media[i][0] == '\0')
            break;

        /* 해당 매체에 장애('1')가 하나라도 있는지 확인 */
        if (sub_CheckValue(g_host_info_shm[g_host_info_idx].TaskHaltStat[i], 50))
        {
            prnt_cnt = 0;
            cprintf("red", "\n[%02d: %-20s]", i, Media[i]);

            for (j = 0; j < 50; j++)
            {
                if (Job[j][0] == '\0')
                    break;
                if (g_host_info_shm[g_host_info_idx].TaskHaltStat[i][j] == '1')
                {
                    if (prnt_cnt % 3 == 0)
                        printf("\n  ");
                    else
                        printf("  ");
                    cprintf("yellow", "<%02d:%-15s>", j, Job[j]);
                    prnt_cnt++;
                }
            }
        }
    }
    printf("\n");

    /* 유량제어 대상 출력 */
    cprintf("sky", "\n<< 유량제어(Flow Control) 상태 >>\n");
    int flow_exists = 0;
    for (i = 0; i < 10; i++)
    {
        if (g_host_info_shm[g_host_info_idx].FlowCtl[i] == '1')
        {
            cprintf("red", "[%d번 활성] ", i);
            flow_exists = 1;
        }
    }
    if (!flow_exists)
        printf("(활성된 유량제어 없음)\n");
    else
        printf("\n");

    /* 전체 매트릭스 출력 */
    print_task_halt_stat_all();
}

/* ----------------------------------------------------------------
 * 테스트 및 메인 함수
 * ---------------------------------------------------------------- */

/**
 * 시뮬레이션을 위한 샘플 데이터를 세팅
 */
void setup_test_data(void)
{
    memset(&g_host_info_shm[g_host_info_idx], 0x00, sizeof(HOST_INFO_SHM));

    /* 기본 정보 설정 */
    memcpy(g_host_info_shm[g_host_info_idx].DealDate, "20260505", 8);
    g_host_info_shm[g_host_info_idx].BankMCAStat[0] = '1';
    g_host_info_shm[g_host_info_idx].ShmLoadYn[0] = 'Y';

    /* 샘플 장애 데이터 입력 */
    update_task_halt_stat(5, 3, '1');  /* IC 자행카드 - 현금지급 */
    update_task_halt_stat(5, 12, '1'); /* IC 자행카드 - 출금이체 */
    update_task_halt_stat(5, 15, '1'); /* IC 자행카드 - 통장정리 */

    update_task_halt_stat(7, 0, '1'); /* RF 자행모바일 - 잔액조회 */
    update_task_halt_stat(7, 8, '1'); /* RF 자행모바일 - 계좌이체 */

    update_task_halt_stat(10, 7, '1');  /* 통장 요구불 - CMS집금 */
    update_task_halt_stat(10, 11, '1'); /* 통장 요구불 - 입금이체 */
    update_task_halt_stat(10, 14, '1'); /* 통장 요구불 - 삼행이체 */

    /* 유량제어 샘플 */
    g_host_info_shm[g_host_info_idx].FlowCtl[2] = '1';
    g_host_info_shm[g_host_info_idx].FlowCtl[5] = '1';
}

int main(void)
{
    /* 종료 시그널 등록 */
    signal(SIGINT, do_exit);
    signal(SIGTERM, do_exit);

    /* 테스트 데이터 준비 및 출력 */
    setup_test_data();
    shm_all_view();

    printf("\n[알림] 조회가 완료되었습니다.\n");
    return 0;
}

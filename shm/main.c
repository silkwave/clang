/*
 * main.c
 * 공유메모리(샘플) 내용을 사람이 보기 좋게 출력하는 예제
 *
 * 아래 코드는 기존 시스템 소스(apviewctrl.c / str_apcmn.h) 일부를
 * 로컬에서 단독 실행 가능하도록 정리한 형태입니다.
 *
 * - 공유메모리 구조체(발췌): HOST_INFO_SHM
 * - 출력 대상: HOST_INFO_SHM::TaskHaltStat, HOST_INFO_SHM::FlowCtl
 *
 * 컴파일: gcc -o main main.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>

/* ================================================================
 * [사진3] str_apcmn.h - 공유메모리 구조체 정의
 * ================================================================ */

#define MAX_HOST_INFO_CNT   2
typedef struct {
    char    JungJoSect   [ 1];      /* 중조구분                        */
    char    DealDate     [ 8];      /* 거래일정보                      */
    char    WkInx        [ 1];      /* 요일인덱스                      */
    char    BankHldYn    [ 1];      /* 금융휴무여부                    */
    char    HldYn        [ 1];      /* 휴일여부                        */
    char    DateBatchYn  [ 1];      /* 일자batch수행여부               */
    char    BankMCAStat  [ 1];      /* 신용MCA상태 1:개국 2:폐국       */
    char    ShmLoadYn    [ 1];      /* SharedMemory 전체 항목 load여부 */
    char    SvrErrYn     [ 1];      /* AP서버 전체업무 장애            */
    char    TaskHaltStat [50][50];  /* 업무별장애상태                  */
    char    FlowCtl      [10];
    char    Filler       [100];
} HOST_INFO_SHM;
#define SZ_HOST_INFO_SHM    (sizeof(HOST_INFO_SHM))

/* ================================================================
 * [사진2] 90~92라인 - 전역 변수
 * ================================================================ */

/* 원본 소스에서 갱신 감지/디버깅 용도로 쓰이던 플래그(이 예제에서는 미사용) */
int  upd_job      = 0, upd_media = 0;
char upd_value    = -1;
int  gi_upd_value = -1;

static int dbflg  = 0;
static int allflg = 0;

/*
 * 공유메모리에서는 보통 (HOST_INFO_SHM *) 형태의 포인터로 접근하지만,
 * 여기서는 "인덱스 방식"으로 접근할 수 있도록 배열 + 인덱스를 사용한다.
 */
static HOST_INFO_SHM g_host_info_shm[MAX_HOST_INFO_CNT];
static int g_host_info_idx = 0;

/* ================================================================
 * [사진2] 94~128라인 - char Media[50][32]
 * EUC-KR: "IS03 M/S 신용겸용" = 17byte → 배열 32로 확장
 * ================================================================ */

char Media[50][32] = {
    "IS03 M/S 신용겸용",    /*  0 */
    "IS03 M/S 중앙카드",    /*  1 */
    "IS03 M/S 조합카드",    /*  2 */
    "IS03 M/S 가상계좌",    /*  3 */
    "IS03 M/S 타행카드",    /*  4 */
    "IC 자행카드",           /*  5 */
    "IC 타행카드",           /*  6 */
    "RF 자행모바일",         /*  7 */
    "RF 타행모바일",         /*  8 */
    "IRFM 모바일",           /*  9 */
    "통장 요구불",           /* 10 */
    "통장 신탁",             /* 11 */
    "통장 저축성",           /* 12 */
    "통장 수익증권",         /* 13 */
    "통장 공제",             /* 14 */
    "전자통장 요구불",       /* 15 */
    "전자통장 신탁",         /* 16 */
    "전자통장 저축성",       /* 17 */
    "전자통장 수익증권",     /* 18 */
    "전자화폐 자행",         /* 19 */
    "전자화폐 타행",         /* 20 */
    "무통장 요구불",         /* 21 */
    "무통장 신탁",           /* 22 */
    "무통장 저축성",         /* 23 */
    "무통장 수익증권",       /* 24 */
    "IS02 M/S 자행",         /* 25 */
    "IS02 M/S 타행",         /* 26 */
    "IS02 M/S 타사",         /* 27 */
    "IS02 M/S 해외",         /* 28 */
    "IS02 EMV 자행",         /* 29 */
    "IS02 EMV 타행",         /* 30 */
    "IS02 EMV 타사",         /* 31 */
    ""                       /* 32: 종료 마커 */
};

/* ================================================================
 * [사진1] 130~161라인 - char Job[50][32]
 * ================================================================ */

char Job[50][32] = {
    "잔액조회",             /*  0 */
    "내역조회",             /*  1 */
    "대금조회",             /*  2 */
    "현금지급",             /*  3 */
    "수표지급",             /*  4 */
    "현금입금",             /*  5 */
    "수표입금",             /*  6 */
    "CMS집금",              /*  7 */
    "계좌이체",             /*  8 */
    "CMS이체",              /*  9 */
    "카드론이체",           /* 10 */
    "입금이체",             /* 11 */
    "출금이체",             /* 12 */
    "동행이체",             /* 13 */
    "삼행이체",             /* 14 */
    "통장정리",             /* 15 */
    "아파트관리비",         /* 16 */
    "공과금수납",           /* 17 */
    "사고신고",             /* 18 */
    "비밀번호변경",         /* 19 */
    "전자화폐충전환불",     /* 20 */
    "신용선결제",           /* 21 */
    "신용청구대금결제",     /* 22 */
    "대학등록금",           /* 23 */
    "공제납부",             /* 24 */
    "여신상환",             /* 25 */
    "지방세",               /* 26 */
    "전자납부",             /* 27 */
    "2D바코드",             /* 28 */
    ""                      /* 29: 종료 마커 */
};

/* ================================================================
 * cprintf - ANSI 컬러 출력 (원본은 별도 라이브러리)
 * ================================================================ */

void cprintf(const char *color, const char *fmt, ...)
{
    va_list ap;
    const char *code = "\033[0m";

    /* color 문자열에 따라 ANSI 색상 코드를 선택 */
    if      (strcmp(color, "red")    == 0) code = "\033[0;31m";
    else if (strcmp(color, "yellow") == 0) code = "\033[0;33m";
    else if (strcmp(color, "sky")    == 0) code = "\033[0;36m";
    else if (strcmp(color, "green")  == 0) code = "\033[0;32m";
    else if (strcmp(color, "white")  == 0) code = "\033[0;37m";

    /* 출력 후에는 항상 기본 색상으로 복귀 */
    printf("%s", code);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\033[0m");
}

/* ================================================================
 * [사진5] 1114~1129라인 - sub_CheckValue
 * 0x00, 0x20(' '), 0x30('0') 이외 값이 있으면 1 반환
 * ================================================================ */

int sub_CheckValue(char *pc_Src, int pi_Len)
{
    int i;

    /* 특정 값(0x00/공백/'0')만 들어있는지 검사한다. */
    for (i = 0; i < pi_Len; i++)
    {
        if (pc_Src[i] != 0x00 &&
            pc_Src[i] != 0x20 &&
            pc_Src[i] != 0x30)
        {
            return(1);
        }
    }
    return(0);
}

/* ================================================================
 * [사진1] 163~166라인 - do_exit
 * ================================================================ */

void do_exit(int sig)
{
    /* 종료 시 필요한 정리 작업(원본: DB 연결 해제 등)을 수행한 뒤 종료 */
    if (dbflg || allflg)
    {
        /* 원본: dbconnect_done(); */
        printf("\n[do_exit] DB 연결 해제 처리\n");
    }
    printf("[do_exit] 프로세스 종료 (sig=%d)\n", sig);
    exit(0);
}

void print_task_halt_stat_all(void)
{
    int i, j;

    if (g_host_info_idx < 0 || g_host_info_idx >= MAX_HOST_INFO_CNT)
    {
        printf("[print_task_halt_stat_all] ERROR: host info index out of range (%d)\n", g_host_info_idx);
        return;
    }

    /* TaskHaltStat 전체값을 50x50 매트릭스로 출력(0/1/X 정규화) */
    cprintf("sky", "\n\n<<TaskHaltStat 전체값(0/1/X)>>\n");
    cprintf("white", "      ");
    for (j = 0; j < 50; j++)
        printf("%d", (j / 10) % 10);
    printf("\n");
    cprintf("white", "      ");
    for (j = 0; j < 50; j++)
        printf("%d", j % 10);
    printf("\n");

    for (i = 0; i < 50; i++)
    {
        const char *media_name = (Media[i][0] != '\0') ? Media[i] : "(미정)";

        cprintf("white", "[%02d] ", i);
        printf("%-20s ", media_name);

        for (j = 0; j < 50; j++)
        {
            unsigned char v = (unsigned char)g_host_info_shm[g_host_info_idx].TaskHaltStat[i][j];

            if (v == 0x00 || v == 0x20 || v == 0x30)
                putchar('0');
            else if (v == '1')
                putchar('1');
            else
                putchar('X');
        }
        putchar('\n');
    }
}

/* ================================================================
 * [사진4] 317~453라인 - shm_all_view
 * ================================================================ */

void shm_all_view(void)
{
    char           buf[256];
    char           tmp[32];
    int            i, j, prnt_cnt;
    int            std_log_on = 0;

    if (g_host_info_idx < 0 || g_host_info_idx >= MAX_HOST_INFO_CNT)
    {
        printf("[shm_all_view] ERROR: host info index out of range (%d)\n", g_host_info_idx);
        return;
    }

    /* 운영에서는 설정에 따라 STD Log 출력 여부가 바뀜(예제에서는 OFF 고정) */
    /* STD Log ON/OFF (405~411라인) */
    if (std_log_on)
        fprintf(stdout, "STD Log (ON)\t");
    else
        cprintf("red", "STD Log (OFF)\t");

    /*
     * TaskHaltStat[매체][업무] = '1' 이면 해당 조합이 장애 상태임을 의미.
     * Media/Job 배열은 인덱스에 대응되는 한글 명칭 표이다.
     */
    /* 매체별 업무장애 (413~440라인) */
#if 1
    cprintf("sky", "\n\n<<매체별 업무장애>>");

    for (i = 0; i < 50; i++)
    {
        if (Media[i][0] == '\0') break;

        /* 해당 매체 행에 의미있는 값이 있는지(전부 0/공백/'0'인지) 먼저 확인 */
        if (sub_CheckValue(g_host_info_shm[g_host_info_idx].TaskHaltStat[i],
                           sizeof(g_host_info_shm[g_host_info_idx].TaskHaltStat[i])))
        {
            prnt_cnt = 0;

            cprintf("red", "\n[%02d:%-20s]", i, Media[i]);

            for (j = 0; j < 50; j++)
            {
                if (Job[j][0] == '\0') break;

                /* '1'로 표시된 업무만 출력 */
                if (g_host_info_shm[g_host_info_idx].TaskHaltStat[i][j] == '1')
                {
                    if (prnt_cnt % 3)
                        fprintf(stdout, "\t");
                    else
                        fprintf(stdout, "\n");

                    cprintf("yellow", "<%02d:%-20s>", j, Job[j]);
                    prnt_cnt++;
                }
            }
        }
    }
#endif

    /* FlowCtl[i] == '1' 인 인덱스를 모아서 한 줄로 출력 */
    /* 유량제어 대상 일련번호 (442~452라인) */
    cprintf("sky", "\n\n<<유량제어 대상 일련번호>>\n");

    memset(buf, 0x00, sizeof(buf));

    for (i = 0; i < 10; i++)
    {
        if (g_host_info_shm[g_host_info_idx].FlowCtl[i] == '1')
        {
            snprintf(tmp, sizeof(tmp), "[%d] ", i);
            strncat(buf, tmp, sizeof(buf) - strlen(buf) - 1);
        }
    }

    if (buf[0] != 0x00)
        cprintf("red", "%s\n", buf);
    else
        printf("(없음)\n");

    print_task_halt_stat_all();

    return;
}

/* ================================================================
 * 테스트 데이터 세팅
 * ================================================================ */

void setup_test_data(void)
{
    /* 실제 시스템에서는 shm attach 결과를 gS_host_info_shm에 연결 */
    memset(&g_host_info_shm[g_host_info_idx], 0x00, sizeof(g_host_info_shm[g_host_info_idx]));

    memcpy(g_host_info_shm[g_host_info_idx].DealDate,    "20260421", 8);
    g_host_info_shm[g_host_info_idx].BankMCAStat[0] = '1';   /* 개국 */
    g_host_info_shm[g_host_info_idx].SvrErrYn[0]    = '0';
    g_host_info_shm[g_host_info_idx].ShmLoadYn[0]   = 'Y';

    /* 샘플 장애 데이터(매체/업무 조합) */
    /* IC 자행카드(5): 현금지급(3), 출금이체(12), 통장정리(15) */
    g_host_info_shm[g_host_info_idx].TaskHaltStat[5][3]   = '1';
    g_host_info_shm[g_host_info_idx].TaskHaltStat[5][12]  = '1';
    g_host_info_shm[g_host_info_idx].TaskHaltStat[5][15]  = '1';

    /* RF 자행모바일(7): 잔액조회(0), 계좌이체(8) */
    g_host_info_shm[g_host_info_idx].TaskHaltStat[7][0]   = '1';
    g_host_info_shm[g_host_info_idx].TaskHaltStat[7][8]   = '1';

    /* 통장 요구불(10): CMS집금(7), 입금이체(11), 삼행이체(14) */
    g_host_info_shm[g_host_info_idx].TaskHaltStat[10][7]  = '1';
    g_host_info_shm[g_host_info_idx].TaskHaltStat[10][11] = '1';
    g_host_info_shm[g_host_info_idx].TaskHaltStat[10][14] = '1';

    /* 유량제어: 2번, 5번 */
    g_host_info_shm[g_host_info_idx].FlowCtl[2] = '1';
    g_host_info_shm[g_host_info_idx].FlowCtl[5] = '1';
}

/* ================================================================
 * sub_CheckValue 단위 테스트
 * ================================================================ */

void test_sub_CheckValue(void)
{
    char buf[10];
    int  ret;

    /* 간단한 입력 패턴으로 sub_CheckValue 기대값을 확인 */
    cprintf("sky", "\n==============================\n");
    cprintf("sky", " sub_CheckValue 단위 테스트\n");
    cprintf("sky", "==============================\n");

    memset(buf, 0x00, sizeof(buf));
    ret = sub_CheckValue(buf, sizeof(buf));
    printf("[케이스1] 0x00 초기화    → %d %s\n", ret, ret==0?"OK":"FAIL");

    memset(buf, 0x20, sizeof(buf));
    ret = sub_CheckValue(buf, sizeof(buf));
    printf("[케이스2] 0x20 공백      → %d %s\n", ret, ret==0?"OK":"FAIL");

    memset(buf, 0x30, sizeof(buf));
    ret = sub_CheckValue(buf, sizeof(buf));
    printf("[케이스3] 0x30 '0'       → %d %s\n", ret, ret==0?"OK":"FAIL");

    memset(buf, 0x00, sizeof(buf));
    buf[3] = '1';
    ret = sub_CheckValue(buf, sizeof(buf));
    printf("[케이스4] '1' 포함       → %d %s\n", ret, ret==1?"OK":"FAIL");

    memset(buf, 0x00, sizeof(buf));
    buf[0] = '2';
    ret = sub_CheckValue(buf, sizeof(buf));
    printf("[케이스5] '2' 포함(폐국) → %d %s\n", ret, ret==1?"OK":"FAIL");
}

/* ================================================================
 * main
 * ================================================================ */

int main(void)
{
    /* Ctrl+C(SIGINT), kill(SIGTERM)로 종료될 때 do_exit 호출 */
    signal(SIGINT,  do_exit);
    signal(SIGTERM, do_exit);

    /* 공유메모리 대신 샘플 데이터를 채워 화면 출력 동작을 확인 */
    setup_test_data();
    test_sub_CheckValue();

    cprintf("sky", "\n==============================\n");
    cprintf("sky", " shm_all_view 실행\n");
    cprintf("sky", "==============================\n");

    shm_all_view();

    printf("\n\n");
    return 0;
}

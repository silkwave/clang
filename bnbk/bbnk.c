/*
 * =========================================================
 * 통장 출력 문자열 생성기 (초보자용, 인덱스 기반)
 * =========================================================
 *
 * 목표
 * - "한 줄(고정 폭 120바이트)" 통장 라인을 만든다.
 * - 포인터를 증가시키는 방식(plc_Char += n) 대신,
 *   offset(정수 인덱스)을 증가시키며 버퍼를 채운다.
 *
 * 구성 요약
 * - BankbookRecord : 출력에 필요한 데이터(문자열)
 * - line_init      : 120바이트 라인을 공백으로 초기화
 * - format_amount  : "12345" -> "12,345" 같은 금액 포맷(필요 시 소수 표시)
 * - append_*       : 필드(문자/금액)를 지정 폭만큼 라인에 추가(인덱스 기반)
 *
 * 주의
 * - field 길이는 "바이트 기준"이다.
 * - UTF-8 한글은 화면 폭과 바이트 폭이 달라 콘솔 정렬이 깨질 수 있다.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* 한 줄 고정 폭 */
#define LINE_SIZE 120

/*
 * 레거시 통장 출력 버퍼 형태
 * - 각 라인은 120바이트 "고정 폭"이며 널 종료가 없다.
 * - 최대 76라인(예시)
 */
typedef struct
{
    char BnbkData[76][LINE_SIZE];
} PRT_BNBK_MSG;

/* 금액 포맷 임시 버퍼 크기 */
#define MAX_AMOUNT_LEN 64

/* 입력 문자열 최대 길이(널 제외) */
#define TRDT_MAX_LEN 8
#define CONTENT_MAX_LEN 20
#define MONEY_MAX_LEN 20

typedef struct
{
    char text[MAX_AMOUNT_LEN];
} AmountString;

/*
 * 초보자용 규칙(중요)
 * - C에서 문자열은 보통 char 배열로 다룬다.
 * - 함수 인자로 "char s[]"라고 써도 내부적으로는 주소(참조)로 전달되지만,
 *   이 소스에서는 문법에 '*'를 노출하지 않고 "배열 + 인덱스" 개념으로 설명한다.
 *
 * 이 프로그램의 핵심 아이디어
 * - 포인터를 움직이지 않는다.
 * - "어디까지 썼는지"를 offset(정수 인덱스)로만 관리한다.
 *   예) offset=0 → 라인 맨 앞, offset=10 → 10칸 사용 완료
 */

/*
 * bounded_strlen
 * - src가 너무 길거나 '\0'이 없을 수도 있으니 최대 max_len까지만 검사한다.
 * - 안전하게 "최대 길이 안에서의 문자열 길이"를 구한다.
 */
static int bounded_strlen(const char s[], int max_len)
{
    if (s == NULL || max_len <= 0)
    {
        return 0;
    }
    for (int i = 0; i < max_len; i++)
    {
        if (s[i] == '\0')
        {
            return i;
        }
    }
    return max_len;
}

/*
 * copy_bounded
 * - src를 dst로 복사하되, src_max_len/ dst_size를 넘지 않게 안전 복사한다.
 * - 마지막에는 항상 '\0'로 문자열을 종료한다.
 */
static void copy_bounded(char dst[], int dst_size, const char src[], int src_max_len)
{
    if (dst == NULL || dst_size <= 0)
    {
        return;
    }
    dst[0] = '\0';
    if (dst_size == 1)
    {
        return;
    }

    int src_len = bounded_strlen(src, src_max_len);
    int copy_len = (src_len < (dst_size - 1)) ? src_len : (dst_size - 1);
    for (int i = 0; i < copy_len; i++)
    {
        dst[i] = src[i];
    }
    dst[copy_len] = '\0';
}

/*
 * DEBUG=1로 빌드하면 내부 동작(필드 추가 과정)을 출력한다.
 * 예: make CFLAGS+=' -DDEBUG=1'
 */
#ifndef DEBUG
#define DEBUG 1
#endif

#if DEBUG
#define DBG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...) ((void)0)
#endif

/* 문자열 우측 공백 제거(콘솔 출력용) */
static void rtrim(char str[])
{
    int len = (int)strlen(str);
    while (len > 0)
    {
        char c = str[len - 1];
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
        {
            str[len - 1] = '\0';
            len--;
            continue;
        }
        break;
    }
}

/*
 * 금액 문자열을 "천단위 쉼표 + 소수 2자리"로 변환
 * - 입력 예: "12345" -> "12,345"      (소수부가 .00 이면 제거)
 * - 입력 예: "9876543.21" -> "9,876,543.21"
 *
 * 제한
 * - atof 기반이므로 비정상 입력은 0.0으로 해석될 수 있다.
 * - locale 미고려(구분자는 ',' 고정).
 */
static AmountString format_amount(const char src[MONEY_MAX_LEN + 1])
{
    AmountString out;
    out.text[0] = '\0';
    int dest_size = (int)sizeof(out.text);

    /*
     * 처리 순서(인덱스 기반)
     * 1) atof + snprintf로 일단 "12345.00"처럼 만든다.
     * 2) '.' 위치(dot_pos)를 인덱스로 찾는다.
     * 3) 소수부가 ".00"이면 제거한다.
     * 4) 정수부를 뒤에서부터(out.text 뒤쪽부터) 채우면서 3자리마다 ','를 넣는다.
     * 5) 소수부가 있으면 그대로 뒤에 붙인다.
     */
    double value = atof(src ? src : "0");
    char temp[MAX_AMOUNT_LEN];
    snprintf(temp, sizeof(temp), "%.2f", value);

    int temp_len = (int)strlen(temp);
    int dot_pos = -1;
    for (int i = 0; i < temp_len; i++)
    {
        if (temp[i] == '.')
        {
            dot_pos = i;
            break;
        }
    }

    /*
     * 소수부가 ".00"이면 통째로 제거해서 "12,345" 형태로 만든다.
     * (요구사항: 소숫점이 없으면 12,345)
     */
    if (dot_pos >= 0 && temp[dot_pos + 1] == '0' && temp[dot_pos + 2] == '0' && temp[dot_pos + 3] == '\0')
    {
        temp[dot_pos] = '\0';
        temp_len = dot_pos;
        dot_pos = -1;
    }

    int int_len = (dot_pos >= 0) ? dot_pos : temp_len;
    int suffix_len = (dot_pos >= 0) ? (temp_len - dot_pos) : 0; /* ".xx" 길이 */
    int comma_count = (int_len > 0) ? ((int_len - 1) / 3) : 0;

    int int_out_len = int_len + comma_count;
    int out_len = int_out_len + suffix_len;
    if (out_len > dest_size - 1)
    {
        out_len = dest_size - 1;
    }
    out.text[out_len] = '\0';

    /*
     * out_len이 너무 작으면(버퍼가 작으면) 앞쪽이 잘릴 수 있다.
     * 이 경우에도 "오른쪽(끝) 기준"으로 금액이 보이도록 만든다.
     */
    int copy_int_len = (out_len < int_out_len) ? out_len : int_out_len;
    int copy_suffix_len = out_len - copy_int_len; /* 0..suffix_len (잘릴 수 있음) */

    /* 1) 소수부를 뒤쪽에 복사 (가능한 만큼) */
    if (dot_pos >= 0 && copy_suffix_len > 0)
    {
        int src_start = dot_pos;
        for (int i = 0; i < copy_suffix_len; i++)
        {
            out.text[copy_int_len + i] = temp[src_start + i];
        }
    }

    /* 2) 정수부를 뒤에서부터 채우면서 3자리마다 쉼표 삽입 */
    int src_idx = int_len - 1;
    int dest_idx = copy_int_len - 1;
    int digit_count = 0;
    while (src_idx >= 0 && dest_idx >= 0)
    {
        out.text[dest_idx--] = temp[src_idx--];
        digit_count++;
        if (digit_count == 3 && src_idx >= 0 && dest_idx >= 0)
        {
            out.text[dest_idx--] = ',';
            digit_count = 0;
        }
    }

    /*
     * 만약 버퍼가 너무 작아서 앞쪽이 남았다면(= 덜 채워졌다면),
     * 빈칸 없이 붙도록 왼쪽을 당긴다.
     */
    if (dest_idx >= 0)
    {
        int shift = dest_idx + 1; /* 실제 문자열 시작 위치 */
        int new_len = out_len - shift;
        for (int i = 0; i < new_len; i++)
        {
            out.text[i] = out.text[shift + i];
        }
        out.text[new_len] = '\0';
    }

    return out;
}

/* 통장 출력 데이터(예시) */
typedef struct
{
    char trDt[9];     /* YYYYMMDD */
    char content[21]; /* 거래내용 */
    char outAmt[21];  /* 출금액(문자열) */
    char inAmt[21];   /* 입금액(문자열) */
    char balance[21]; /* 잔액(문자열) */
} BankbookRecord;

static void line_init(char buffer[LINE_SIZE + 1])
{
    /* 120칸을 공백으로 깔아두면, 나중에 일부만 채워도 "고정 폭"이 유지된다. */
    memset(buffer, ' ', LINE_SIZE);
    buffer[LINE_SIZE] = '\0';
}

static void bnbk_msg_init(PRT_BNBK_MSG *msg)
{
    if (msg == NULL)
    {
        return;
    }
    /* 레거시 버퍼는 라인마다 널 종료가 없으므로 전체를 공백으로 초기화 */
    memset(msg->BnbkData, ' ', sizeof(msg->BnbkData));
}

/*
 * append_char_field
 * - 글자 필드를 field_len 폭만큼 "좌측 정렬"로 추가한다.
 * - 남은 공간이 부족하면 남은 만큼만 기록
 *
 * 리턴값
 * - "새 offset"을 리턴한다. (offset += write_len 한 결과)
 */
static int append_char_field(char buffer[LINE_SIZE + 1], int offset, const char src[], int field_len)
{
    if (offset >= LINE_SIZE || field_len <= 0)
    {
        return offset;
    }

    int remain = LINE_SIZE - offset;
    int write_len = field_len < remain ? field_len : remain;

    int src_len = bounded_strlen(src, CONTENT_MAX_LEN);
    int copy_len = (src_len < write_len) ? src_len : write_len;

    /*
     * 인덱스 방식으로 좌측 정렬:
     * - field(폭 write_len) 전체를 공백으로 채움
     * - 앞에서부터 copy_len 만큼만 src를 복사
     */
    char field[LINE_SIZE + 1]; /* write_len은 LINE_SIZE(120) 이하 */
    for (int i = 0; i < write_len; i++)
    {
        field[i] = ' ';
    }
    for (int i = 0; i < copy_len; i++)
    {
        field[i] = src[i];
    }
    for (int i = 0; i < write_len; i++)
    {
        buffer[offset + i] = field[i];
    }

    DBG_PRINTF("[append C] off=%d len=%d src=\"%s\"\n", offset, write_len, src ? src : "(null)");

    return offset + write_len;
}

/*
 * append_amount_field
 * - 금액 필드를 field_len 폭만큼 "우측 정렬"로 추가한다.
 * - 남은 공간이 부족하면 남은 만큼만 기록
 *
 * 리턴값
 * - "새 offset"을 리턴한다. (offset += write_len 한 결과)
 */
static int append_amount_field(char buffer[LINE_SIZE + 1], int offset, const char src[], int field_len)
{
    if (offset >= LINE_SIZE || field_len <= 0)
    {
        return offset;
    }

    int remain = LINE_SIZE - offset;
    int write_len = field_len < remain ? field_len : remain;

    char money_src[MONEY_MAX_LEN + 1];
    copy_bounded(money_src, (int)sizeof(money_src), src ? src : "0", MONEY_MAX_LEN);
    AmountString amount = format_amount(money_src);
    int amount_len = (int)strlen(amount.text);
    /*
     * 인덱스 방식으로 우측 정렬:
     * - field(폭 write_len) 전체를 공백으로 채움
     * - amount가 길면 오른쪽(write_len)만큼만 잘라서 표시
     * - amount가 짧으면 오른쪽 정렬로 배치
     */
    char field[LINE_SIZE + 1]; /* write_len은 LINE_SIZE(120) 이하 */
    for (int i = 0; i < write_len; i++)
    {
        field[i] = ' ';
    }

    if (amount_len > 0)
    {
        if (amount_len >= write_len)
        {
            int src_start = amount_len - write_len;
            for (int i = 0; i < write_len; i++)
            {
                field[i] = amount.text[src_start + i];
            }
        }
        else
        {
            int dest_start = write_len - amount_len;
            for (int i = 0; i < amount_len; i++)
            {
                field[dest_start + i] = amount.text[i];
            }
        }
    }

    for (int i = 0; i < write_len; i++)
    {
        buffer[offset + i] = field[i];
    }

    DBG_PRINTF("[append F] off=%d len=%d src=\"%s\" fmt=\"%s\"\n",
               offset, write_len, src ? src : "(null)", amount.text);

    return offset + write_len;
}

/*
 * append_bytes
 * - 레거시의 memcpy(plc_Char, ..., n); plc_Char += n; 을
 *   "offset 증가" 방식으로 표현한다.
 */
static int append_bytes(char buffer[LINE_SIZE + 1], int offset, const unsigned char bytes[], int byte_len)
{
    if (buffer == NULL || bytes == NULL || offset >= LINE_SIZE || byte_len <= 0)
    {
        return offset;
    }

    int remain = LINE_SIZE - offset;
    int write_len = (byte_len < remain) ? byte_len : remain;
    for (int i = 0; i < write_len; i++)
    {
        buffer[offset + i] = (char)bytes[i];
    }
    return offset + write_len;
}

/*
 * 통장 한 줄 생성
 * - outLine은 LINE_SIZE + 1 크기의 버퍼(배열)여야 한다.
 */
static void make_bankbook_line(BankbookRecord record, char outLine[LINE_SIZE + 1])
{
    char line[LINE_SIZE + 1];
    line_init(line);
    int offset = 0;

    /* 레거시 예시:
     * memcpy(plc_Char, "\xff\x00\x01\x00", 4);
     * plc_Char += 4;
     */
    static const unsigned char ctrl_prefix[] = {0xff, 0x00, 0x01, 0x00};
    offset = append_bytes(line, offset, ctrl_prefix, (int)sizeof(ctrl_prefix));

    /* 레이아웃(필드 폭): 날짜(10) + 내용(20) + 출금(15) + 입금(15) + 잔액(20) */
    offset = append_char_field(line, offset, record.trDt, 10);
    offset = append_char_field(line, offset, record.content, 20);
    offset = append_amount_field(line, offset, record.outAmt, 15);
    offset = append_amount_field(line, offset, record.inAmt, 15);
    offset = append_amount_field(line, offset, record.balance, 20);

    (void)offset; /* 남은 칸은 공백이며, offset은 디버그/학습용 */
    memcpy(outLine, line, LINE_SIZE + 1);
}

/* record 내용 출력(검증용) */
static void print_record(BankbookRecord record)
{
    printf("[record]\n");
    printf("  trDt    : %s\n", record.trDt);
    printf("  content : %s\n", record.content);
    printf("  outAmt  : %s\n", record.outAmt);
    printf("  inAmt   : %s\n", record.inAmt);
    printf("  balance : %s\n", record.balance);
}

int main(void)
{
    /*
     * 예제 데이터 여러 개
     * - 실제로는 DB/파일/네트워크에서 읽어 record를 채운 뒤 동일하게 처리하면 된다.
     */
    BankbookRecord records[3];
    memset(records, 0x00, sizeof(records));

    strcpy(records[0].trDt, "20260511");
    strcpy(records[0].content, "ATM출금");
    strcpy(records[0].outAmt, "12345");
    strcpy(records[0].inAmt, "0");
    strcpy(records[0].balance, "9876543.21");

    strcpy(records[1].trDt, "20260512");
    strcpy(records[1].content, "급여입금");
    strcpy(records[1].outAmt, "0");
    strcpy(records[1].inAmt, "2500000");
    strcpy(records[1].balance, "12376543.21");

    strcpy(records[2].trDt, "20260513");
    strcpy(records[2].content, "카드결제");
    strcpy(records[2].outAmt, "45678.9");
    strcpy(records[2].inAmt, "0");
    strcpy(records[2].balance, "12330864.31");

    /*
     * 레거시 PRT_BNBK_MSG 형태로 여러 줄을 담는다.
     * - msg.BnbkData[i]는 "i번째 라인(120바이트)" 버퍼
     */
    PRT_BNBK_MSG msg;
    bnbk_msg_init(&msg);

    int record_count = (int)(sizeof(records) / sizeof(records[0]));
    for (int i = 0; i < record_count; i++)
    {
        /*
         * =========================================================
         * 레거시의 "BnbkData[i]"에 해당하는 부분
         * =========================================================
         *
         * 레거시:
         *   plc_Char = ps_prt_bnbk_msg->BnbkData[i];
         *
         * 현재:
         *   msg.BnbkData[i]가 "i번째 라인(120바이트)" 버퍼의 시작 주소 역할을 한다.
         */
        /* 필요하면 개별 record 내용을 먼저 확인 */
        print_record(records[i]);

        char line[LINE_SIZE + 1];
        make_bankbook_line(records[i], line);
        memcpy(msg.BnbkData[i], line, LINE_SIZE);
    }

    printf("=================================================\n");
    for (int i = 0; i < record_count; i++)
    {
        /* 레거시 라인은 널 종료가 없으므로, 출력용으로만 문자열 버퍼를 만든다. */
        char printable[LINE_SIZE + 1];
        memcpy(printable, msg.BnbkData[i], LINE_SIZE);
        printable[LINE_SIZE] = '\0';
        /* 제어문자(0x00 등)로 콘솔 출력이 깨질 수 있어, 화면용으로 치환 */
        for (int k = 0; k < LINE_SIZE; k++)
        {
            unsigned char c = (unsigned char)printable[k];
            if (c == 0x00 || c == 0xff)
            {
                printable[k] = '.';
            }
        }
        rtrim(printable);
        printf("%s\n", printable);
    }
    printf("=================================================\n");

    return 0;
}

/*
 * =============================================================================
 * 통장 출력 문자열 생성기 (초보자 학습용 - 리팩토링 버전)
 * =============================================================================
 * 
 * [학습 포인트]
 * 1. 배열과 인덱스: 포인터 대신 정수(offset)를 사용하여 위치를 관리합니다.
 * 2. 고정 폭 데이터: 모든 데이터 필드가 정해진 바이트 길이를 가지도록 맞춥니다.
 * 3. 금액 포맷팅: 숫자를 천 단위 쉼표가 들어간 가독성 좋은 문자열로 바꿉니다.
 * 4. 안전한 프로그래밍: 버퍼의 크기를 항상 체크하여 오버플로우를 방지합니다.
 *
 * [주요 규칙]
 * - 모든 출력 라인은 정확히 120바이트여야 합니다.
 * - 데이터가 필드보다 짧으면 공백(' ')으로 채우고, 길면 자릅니다.
 * - 금액은 우측 정렬, 일반 텍스트는 좌측 정렬합니다.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- 설정값 (매크로) --- */

/* 통장 한 줄의 총 길이 (120바이트) */
#define LINE_SIZE 120

/* 금액 처리를 위한 임시 버퍼 크기 */
#define MAX_AMOUNT_LEN 64

/* 입력 데이터들의 최대 허용 길이 */
#define MONEY_MAX_LEN 20
#define CONTENT_MAX_LEN 20

/* 
 * 디버그 모드 설정 (1이면 실행 과정을 화면에 출력) 
 * 실제 배포 시에는 0으로 설정하거나 컴파일 시 -DDEBUG=0 옵션을 줍니다.
 */
#ifndef DEBUG
#define DEBUG 1
#endif

#if DEBUG
#define DBG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...) ((void)0)
#endif

/* --- 구조체 정의 --- */

/* 
 * 통장 1줄의 실제 레거시 저장 형태 
 * 76개의 라인이 있고, 각 라인은 120바이트 크기의 문자 배열입니다.
 */
typedef struct {
    char BnbkData[76][LINE_SIZE];
} PRT_BNBK_MSG;

/* 금액 포맷팅 결과를 담기 위한 바구니 */
typedef struct {
    char text[MAX_AMOUNT_LEN];
} AmountString;

/* 사용자가 입력하는 원본 데이터 구조 */
typedef struct {
    char trDt[9];     /* 거래일자 (YYYYMMDD) */
    char content[21]; /* 거래내용 (예: ATM출금) */
    char outAmt[21];  /* 출금액 */
    char inAmt[21];   /* 입금액 */
    char balance[21]; /* 잔액 */
} BankbookRecord;

/* --- 도우미 함수 (Helper Functions) --- */

/*
 * [함수] get_safe_length
 * [설명] 문자열이 유효한지 확인하고, 지정된 최대 길이를 넘지 않는 실제 길이를 반환합니다.
 */
static int get_safe_length(const char source[], int max_limit) {
    if (source == NULL || max_limit <= 0) return 0;

    for (int i = 0; i < max_limit; i++) {
        if (source[i] == '\0') return i; /* 널 문자를 만나면 그 전까지가 길이 */
    }
    return max_limit; /* 널 문자가 없으면 최대 제한치 반환 */
}

/*
 * [함수] copy_safely
 * [설명] 안전하게 문자열을 복사합니다. 목적지 버퍼 크기를 절대 넘지 않으며 항상 '\0'로 끝냅니다.
 */
static void copy_safely(char destination[], int dest_size, const char source[], int source_max) {
    if (destination == NULL || dest_size <= 0) return;

    /* 초기화 */
    destination[0] = '\0';
    if (dest_size == 1) return;

    int actual_len = get_safe_length(source, source_max);
    /* 목적지 버퍼 크기 - 1 (널 문자 공간 제외) 보다 작아야 함 */
    int copy_count = (actual_len < (dest_size - 1)) ? actual_len : (dest_size - 1);

    for (int i = 0; i < copy_count; i++) {
        destination[i] = source[i];
    }
    destination[copy_count] = '\0'; /* 마지막에 반드시 널 문자 추가 */
}

/*
 * [함수] remove_trailing_spaces
 * [설명] 문자열 오른쪽 끝에 붙은 공백이나 줄바꿈 문자를 제거합니다. (화면 출력용)
 */
static void remove_trailing_spaces(char text[]) {
    int length = (int)strlen(text);
    while (length > 0) {
        char last_char = text[length - 1];
        if (last_char == ' ' || last_char == '\n' || last_char == '\r' || last_char == '\t') {
            text[length - 1] = '\0';
            length--;
        } else {
            break;
        }
    }
}

/* --- 핵심 로직 함수 --- */

/*
 * [함수] format_amount
 * [설명] "12345" 같은 숫자 문자열을 "12,345" 처럼 바꿉니다.
 * [과정] 
 * 1. 실릿수(double)로 변환 후 소수점 2자리 문자열로 만듭니다.
 * 2. 소수점이 .00 이면 정수형으로 취급하여 소수부를 버립니다.
 * 3. 정수 부분을 뒤에서부터 한 글자씩 채우면서 3글자마다 쉼표(,)를 넣습니다.
 */
static AmountString format_amount(const char amount_src[]) {
    AmountString result;
    result.text[0] = '\0';

    /* 1. 숫자로 변환 (입력이 없으면 "0"으로 처리) */
    double numeric_value = atof(amount_src ? amount_src : "0");
    char formatted_temp[MAX_AMOUNT_LEN];
    snprintf(formatted_temp, sizeof(formatted_temp), "%.2f", numeric_value);

    int temp_len = (int)strlen(formatted_temp);
    int dot_position = -1;

    /* 소수점(.) 위치 찾기 */
    for (int i = 0; i < temp_len; i++) {
        if (formatted_temp[i] == '.') {
            dot_position = i;
            break;
        }
    }

    /* 2. 소수점이 .00 으로 끝나면 제거 로직 */
    if (dot_position >= 0 && formatted_temp[dot_position + 1] == '0' && 
        formatted_temp[dot_position + 2] == '0' && formatted_temp[dot_position + 3] == '\0') {
        formatted_temp[dot_position] = '\0';
        temp_len = dot_position;
        dot_position = -1;
    }

    int integer_part_len = (dot_position >= 0) ? dot_position : temp_len;
    int suffix_len = (dot_position >= 0) ? (temp_len - dot_position) : 0; 
    int comma_count = (integer_part_len > 0) ? ((integer_part_len - 1) / 3) : 0;

    /* 최종적으로 만들어질 문자열의 길이 계산 */
    int final_len = integer_part_len + comma_count + suffix_len;
    if (final_len >= MAX_AMOUNT_LEN) final_len = MAX_AMOUNT_LEN - 1;
    
    result.text[final_len] = '\0';

    /* 3. 뒤에서부터 채우기 (우측 정렬 효과) */
    int dest_index = final_len - 1;

    /* A. 소수부 먼저 복사 (있을 경우) */
    if (dot_position >= 0 && suffix_len > 0) {
        for (int i = suffix_len - 1; i >= 0; i--) {
            result.text[dest_index--] = formatted_temp[dot_position + i];
        }
    }

    /* B. 정수부 복사하면서 쉼표 넣기 */
    int source_index = integer_part_len - 1;
    int digit_counter = 0;
    while (source_index >= 0 && dest_index >= 0) {
        result.text[dest_index--] = formatted_temp[source_index--];
        digit_counter++;

        /* 3자리마다 쉼표 삽입 (단, 맨 앞자리일 때는 넣지 않음) */
        if (digit_counter == 3 && source_index >= 0 && dest_index >= 0) {
            result.text[dest_index--] = ',';
            digit_counter = 0;
        }
    }

    return result;
}

/*
 * [함수] append_text_field
 * [설명] 고정 폭 버퍼에 텍스트를 "좌측 정렬"로 추가합니다.
 * [반환] 데이터가 추가된 후의 다음 인덱스(offset)
 */
static int append_text_field(char buffer[], int offset, const char source[], int field_width) {
    if (offset >= LINE_SIZE || field_width <= 0) return offset;

    /* 쓸 수 있는 공간 확인 */
    int remaining_space = LINE_SIZE - offset;
    int write_width = (field_width < remaining_space) ? field_width : remaining_space;

    /* 원본 데이터 길이 확인 */
    int source_len = get_safe_length(source, CONTENT_MAX_LEN);
    int copy_len = (source_len < write_width) ? source_len : write_width;

    /* 1. 필드 전체를 먼저 공백으로 채움 (초기화) */
    for (int i = 0; i < write_width; i++) {
        buffer[offset + i] = ' ';
    }

    /* 2. 앞에서부터(좌측 정렬) 데이터 복사 */
    for (int i = 0; i < copy_len; i++) {
        buffer[offset + i] = source[i];
    }

    DBG_PRINTF("[TEXT] 위치: %3d, 폭: %2d, 데이터: \"%s\"\n", offset, write_width, source ? source : "");

    return offset + write_width;
}

/*
 * [함수] append_amount_field
 * [설명] 고정 폭 버퍼에 금액을 "우측 정렬"로 추가합니다.
 */
static int append_amount_field(char buffer[], int offset, const char source[], int field_width) {
    if (offset >= LINE_SIZE || field_width <= 0) return offset;

    int remaining_space = LINE_SIZE - offset;
    int write_width = (field_width < remaining_space) ? field_width : remaining_space;

    /* 금액 포맷팅 (쉼표 넣기) */
    AmountString formatted = format_amount(source);
    int amount_len = (int)strlen(formatted.text);

    /* 1. 필드 전체 공백 채움 */
    for (int i = 0; i < write_width; i++) {
        buffer[offset + i] = ' ';
    }

    /* 2. 우측 정렬 배치 */
    if (amount_len > 0) {
        if (amount_len >= write_width) {
            /* 데이터가 필드보다 길면 오른쪽 부분을 잘라서 넣음 */
            int start_from = amount_len - write_width;
            for (int i = 0; i < write_width; i++) {
                buffer[offset + i] = formatted.text[start_from + i];
            }
        } else {
            /* 데이터가 필드보다 짧으면 오른쪽 끝에 붙임 */
            int start_at = write_width - amount_len;
            for (int i = 0; i < amount_len; i++) {
                buffer[offset + start_at + i] = formatted.text[i];
            }
        }
    }

    DBG_PRINTF("[MONY] 위치: %3d, 폭: %2d, 원본: \"%s\", 결과: \"%s\"\n", 
               offset, write_width, source ? source : "0", formatted.text);

    return offset + write_width;
}

/*
 * [함수] append_raw_bytes
 * [설명] 제어 문자 같은 특수한 바이트 데이터를 그대로 추가합니다.
 */
static int append_raw_bytes(char buffer[], int offset, const unsigned char bytes[], int length) {
    if (buffer == NULL || bytes == NULL || offset >= LINE_SIZE || length <= 0) return offset;

    int remaining = LINE_SIZE - offset;
    int write_len = (length < remaining) ? length : remaining;

    for (int i = 0; i < write_len; i++) {
        buffer[offset + i] = (char)bytes[i];
    }
    return offset + write_len;
}

/* --- 메인 처리 흐름 --- */

/*
 * [함수] make_bankbook_line
 * [설명] 레코드를 받아 120바이트 고정 폭의 통장 출력용 라인 한 줄을 완성합니다.
 */
static void make_bankbook_line(BankbookRecord record, char output_line[]) {
    char temp_line[LINE_SIZE + 1];
    
    /* 1. 라인 초기화 (전체 공백) */
    memset(temp_line, ' ', LINE_SIZE);
    temp_line[LINE_SIZE] = '\0';

    int current_offset = 0;

    /* 2. 제어 문자 추가 (4바이트) */
    static const unsigned char ctrl_code[] = {0xff, 0x00, 0x01, 0x00};
    current_offset = append_raw_bytes(temp_line, current_offset, ctrl_code, (int)sizeof(ctrl_code));

    /* 3. 각 필드 추가 (레이아웃 정의) */
    current_offset = append_text_field(temp_line, current_offset, record.trDt, 10);      /* 날짜 */
    current_offset = append_text_field(temp_line, current_offset, record.content, 20);   /* 내용 */
    current_offset = append_amount_field(temp_line, current_offset, record.outAmt, 15);  /* 출금 */
    current_offset = append_amount_field(temp_line, current_offset, record.inAmt, 15);   /* 입금 */
    current_offset = append_amount_field(temp_line, current_offset, record.balance, 20); /* 잔액 */

    /* 최종 결과 복사 */
    memcpy(output_line, temp_line, LINE_SIZE);
}

int main(void) {
    /* 1. 테스트용 데이터 준비 */
    BankbookRecord records[3] = {
        {"20260511", "ATM출금", "12345", "0", "9876543.21"},
        {"20260512", "급여입금", "0", "2500000", "12376543.21"},
        {"20260513", "카드결제", "45678.9", "0", "12330864.31"}
    };

    /* 2. 레거시 출력 버퍼 초기화 */
    PRT_BNBK_MSG msg_buffer;
    memset(msg_buffer.BnbkData, ' ', sizeof(msg_buffer.BnbkData));

    printf("--- 통장 데이터 생성 시작 ---\n");

    /* 3. 각 레코드를 통장 라인으로 변환 */
    for (int i = 0; i < 3; i++) {
        char finished_line[LINE_SIZE + 1];
        make_bankbook_line(records[i], finished_line);
        
        /* 생성된 120바이트를 버퍼의 i번째 줄에 저장 */
        memcpy(msg_buffer.BnbkData[i], finished_line, LINE_SIZE);
    }

    /* 4. 결과 출력 (검증용) */
    printf("\n--- 최종 출력 결과 (120바이트 고정 폭) ---\n");
    printf("========================================================================================================================\n");
    for (int i = 0; i < 3; i++) {
        char printable[LINE_SIZE + 1];
        memcpy(printable, msg_buffer.BnbkData[i], LINE_SIZE);
        printable[LINE_SIZE] = '\0';

        /* 제어 문자만 '.'으로 바꿔서 출력 (한글 UTF-8은 유지) */
        for (int k = 0; k < LINE_SIZE; k++) {
            unsigned char c = (unsigned char)printable[k];
            /* 0x00~0x1F(제어문자), 0x7F(DEL), 0xFF 등만 마스킹 */
            if (c < 32 || c == 127 || c == 255) printable[k] = '.'; 
        }

        remove_trailing_spaces(printable);
        printf("[%02d] %s\n", i + 1, printable);
    }
    printf("========================================================================================================================\n");

    return 0;
}

/**
 * libcmn_KSCLR - EUC-KR 문자열 끝에서 잘린 한글 바이트 제거
 *
 * EUC-KR 한글은 2바이트 쌍으로 구성된다.
 * 고정 길이 필드에서 버퍼 경계로 한글이 절반만 잘렸을 경우,
 * 마지막 바이트를 공백(' ')으로 교체하여 깨진 문자를 방지한다.
 *
 * @param buf     처리할 문자열 버퍼 (NULL 불가)
 * @param len     버퍼의 유효 데이터 길이
 *                ※ 호출자는 buf[len]까지 쓸 수 있도록 len+1 이상 확보할 것
 */
#include <string.h>

void libcmn_KSCLR(char *buf, int len)
{
    int i;
    int high_bit_count = 0;  /* MSB=1 인 바이트 수 (EUC-KR 한글 바이트 카운트) */

    /* 방어 코드 */
    if (buf == NULL || len <= 0) {
        return;
    }

    /* MSB(0x80)가 세트된 바이트 수를 센다.
     * EUC-KR 한글은 선행/후행 바이트 모두 MSB=1 이므로,
     * 이 카운트가 홀수면 마지막 한글이 1바이트 잘린 것이다. */
    for (i = 0; i < len; i++) {
        if ((unsigned char)buf[i] & 0x80) {
            high_bit_count++;
        }
    }

    /* 홀수 → 마지막 바이트가 잘린 한글의 선행 바이트
     * 공백으로 덮어써서 깨진 문자 방지 */
    if (high_bit_count % 2 != 0) {
        buf[len - 1] = ' ';
    }

    /* NULL 종료 (호출자가 buf[len] 공간을 보장해야 함) */
    buf[len] = '\0';
}


/* ============================================================
 * libcmn_KSALPHA
 * EUC-KR 전문 버퍼 → 전각 영숫자/특수문자 반각 변환
 * 한글은 원형 유지
 * ============================================================ */

/* 전각 선행바이트 */
#define LEAD_ALPHA      0xa3    /* 전각 영숫자 (Ａ～Ｚ, ０～９) */
#define LEAD_SPECIAL    0xa1    /* 전각 특수문자                 */

/* 전각 특수문자 후행바이트 */
#define TRAIL_SPACE     0xa1    /* 전각 공백   → 0x20 ' '       */
#define TRAIL_TILDE     0xad    /* 전각 물결표 → 0x7e '~'       */

/* 전각 영숫자 반각 변환: 후행바이트 - 0x80 = 반각 ASCII
 * 예) 전각'A' 0xa3c1 → 0xc1 - 0x80 = 0x41 = 'A'            */
#define TO_HALF(trail)  ((unsigned char)((trail) - 0x80))


/* ------------------------------------------------------------
 * 전각 특수문자(선행 0xa1) 후행바이트 → 반각 매핑
 * 반환값: 출력한 바이트 수
 * ------------------------------------------------------------ */
static int convert_special(unsigned char trail,
                            unsigned char *pc_Outbuf,
                            int           *pi_OutIdx)
{
    switch (trail) {
        case TRAIL_SPACE:                           /* 전각 공백  */
            pc_Outbuf[(*pi_OutIdx)++] = 0x20;
            return 1;

        case TRAIL_TILDE:                           /* 전각 물결표 */
            pc_Outbuf[(*pi_OutIdx)++] = 0x7e;
            return 1;

        default:                                    /* 변환 불가 → 원형 유지 */
            pc_Outbuf[(*pi_OutIdx)++] = LEAD_SPECIAL;
            pc_Outbuf[(*pi_OutIdx)++] = trail;
            return 2;
    }
}


/* ------------------------------------------------------------
 * libcmn_KSALPHA
 *   pc_Inbuf  : 입력 EUC-KR 바이트 버퍼
 *   pi_ILen   : 처리할 입력 길이
 *   rc_Outbuf : 출력 버퍼 (호출자가 pi_ILen 이상 확보)
 * ------------------------------------------------------------ */
void libcmn_KSALPHA(unsigned char *pc_Inbuf,
                    int            pi_ILen,
                    unsigned char *rc_Outbuf)
{
    int           i      = 0;
    int           outIdx = 0;
    unsigned char c;
    unsigned char trail;

    /* 출력버퍼 공백 초기화 */
    memset(rc_Outbuf, 0x20, pi_ILen);

    while (i < pi_ILen) {

        c = pc_Inbuf[i++];

        if (c >= 0xa0) {
            /* ── 한글 / 전각 영역 (2바이트) ──────────────── */
            trail = pc_Inbuf[i++];

            if (c == LEAD_ALPHA) {
                /* ① 전각 영숫자 → 반각 1바이트 */
                rc_Outbuf[outIdx++] = TO_HALF(trail);

            } else if (c == LEAD_SPECIAL) {
                /* ② 전각 특수문자 → 개별 매핑 */
                convert_special(trail, rc_Outbuf, &outIdx);

            } else {
                /* ③ 일반 한글 → 2바이트 원형 유지 */
                rc_Outbuf[outIdx++] = c;
                rc_Outbuf[outIdx++] = trail;
            }

        } else {
            /* ── ASCII 영역 → 1바이트 그대로 ─────────────── */
            rc_Outbuf[outIdx++] = c;
        }
    }
}


/* ============================================================
 * 사용 예제 (빌드 시에만 포함)
 *
 *   cc -DLIBCMN_EXAMPLE -o example main.c
 *   ./example
 * ============================================================ */

#include <stdio.h>

static void dump_hex(const unsigned char *buf, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf("%02X%s", buf[i], (i + 1 == len) ? "" : " ");
    }
    printf("\n");
}

int main(void)
{
    /* 1) libcmn_KSCLR: 고정 길이 필드에서 한글 1바이트가 잘렸을 때 복구 */
    /* 예시: "가"(B0 A1) + 잘린 선행바이트(B0) */
    {
        unsigned char raw[] = { 0xB0, 0xA1, 0xB0, 0x00 };
        int len = 3;

        printf("[KSCLR] before: ");
        dump_hex(raw, len);

        libcmn_KSCLR((char *)raw, len);

        printf("[KSCLR] after : ");
        dump_hex(raw, len);
        printf("[KSCLR] as str: '%s'\n\n", (char *)raw);
    }

    /* 2) libcmn_KSALPHA: 전각 영숫자/일부 특수문자만 반각으로 변환 */
    {
        /* Ａ( A3 C1 ) ０( A3 B0 ) 전각공백( A1 A1 ) ～( A1 AD ) "가"( B0 A1 ) '-' */
        unsigned char in[] = {
            0xA3, 0xC1,
            0xA3, 0xB0,
            0xA1, 0xA1,
            0xA1, 0xAD,
            0xB0, 0xA1,
            0x2D
        };
        int inLen = (int)sizeof(in);
        unsigned char out[sizeof(in) + 1];

        printf("[KSALPHA] in   : ");
        dump_hex(in, inLen);

        libcmn_KSALPHA(in, inLen, out);
        out[inLen] = 0x00;

        printf("[KSALPHA] out  : ");
        dump_hex(out, inLen);
        printf("[KSALPHA] as str: '%s'\n", (char *)out);
    }

    return 0;
}


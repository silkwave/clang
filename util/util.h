#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIZE 10
#define MAX_DIGIT 20


/**
 * @brief 숫자 형식의 문자열을 안전하게 정수(int)로 변환
 * @param src      읽기 전용 문자열 포인터
 * @param len      변환을 요청할 길이
 * @return int     변환된 정수 값
 */
static inline int cdcd_mk_int(const char *src, int len)
{
    char tmp[MAX_SIZE + 1] = {0};
    int cp_len;

    // 버퍼 오버플로우 방어
    cp_len = (len > MAX_SIZE) ? MAX_SIZE : len;    

    memcpy(tmp, src, cp_len);

    return (int)strtol(tmp, NULL, 10);
}

/**
 * @brief 숫자 형식의 문자열을 안전하게 double(실수)로 변환
 * @param src      읽기 전용 문자열 포인터
 * @param len      변환을 요청할 길이
 * @return double  변환된 정밀 실수 값
 */
static inline double cdcd_mk_double(const char *src, int len)
{
    char buf[MAX_DIGIT + 1] = {0};
    int cp_len;

    // 버퍼 오버플로우 방어
    cp_len = (len > MAX_DIGIT) ? MAX_DIGIT : len;

    memcpy(buf, src, cp_len);

    return strtod(buf, NULL);
}

/**
 * @brief 금액 문자열의 선행 공백, 널, '0'을 제거하고 좌측 정렬
 * @param buf      가변 문자열 포인터
 * @param len      처리 대상 데이터의 길이
 * @return int     남은 유효 문자열의 길이
 */
static inline int cdcd_lspace_zero_amt_trim(char *buf, int len)
{
    int idx;

    if (len > MAX_SIZE)
    {
        len = MAX_SIZE;
    }

    for (idx = 0; idx < len; idx++)
    {
        if (buf[idx] != 0x20 && buf[idx] != 0x00 && buf[idx] != 0x30)
        {
            break;
        }
    }

    if (idx < len && buf[idx] == '.')
    {
        if (idx > 0)
        {
            idx -= 1;
        }
    }

    if (idx == len)
    {
        memset(buf, 0x00, len);
        return 0;
    }
    else
    {
        int rem_len = len - idx;

        if (idx > 0)
        {
            memmove(buf, buf + idx, rem_len);
            memset(buf + rem_len, 0x00, idx);
        }
        return rem_len;
    }
}


/**
 * @brief 금액 문자열을 우측 정렬 및 천단위 콤마 형식으로 가공
 * @param amt_buf  [In/Out] 금액 문자열 버퍼
 * @param out_len  [In]     출력할 최종 고정 길이
 * @param edt_chr  [In]     양수일 때 금액 맨 앞에 채워줄 문자 (예: ' ', '+')
 * @return int       변환 완료된 최종 문자열 길이 (out_len 반환)
 */
static inline int cdcd_make_amt(char *amt_buf, int out_len, char edt_chr)
{
    char fmt[16] = {0x00,};
    char tmp[32] = {0x00,};
    char buf[32] = {0x00,};

    double val;
    int src_pos;
    int dst_pos;
    int comma;

    if (out_len >= (int)sizeof(buf))
    {
        memset(buf, '*', sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = 0x00;
    }
    else
    {
        // 내부 안전 변환 함수 호출
        val = cdcd_mk_double(amt_buf, 15);

        // 포맷 및 임시 문자열 생성
        sprintf(fmt, "%%%d.0lf", out_len);
        sprintf(tmp, fmt, val);

        // 위치 인덱스 및 콤마 카운터 초기화 (모두 10자 이하 단어로 구성)
        src_pos = (int)strlen(tmp) - 1;
        dst_pos = (int)strlen(tmp) - 1;
        comma = 0;

        while (src_pos >= 0)
        {
            if (tmp[src_pos] == ' ')
            {
                break;
            }

            if (tmp[src_pos] == '-')
            {
                buf[dst_pos--] = tmp[src_pos--];
            }
            else
            {
                if (comma != 0 && (comma % 3) == 0)
                {
                    buf[dst_pos--] = ',';
                }
                buf[dst_pos--] = tmp[src_pos--];
                comma++;
            }
        }

        if (buf[dst_pos + 1] != '-')
        {
            buf[dst_pos--] = edt_chr;
        }

        while (dst_pos >= 0)
        {
            buf[dst_pos--] = ' ';
        }

        buf[out_len] = 0x00;
    }

    strcpy(amt_buf, buf);

    return out_len;
}

#endif // UTIL_H
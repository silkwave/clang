#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX_DIGIT
#define MAX_DIGIT 20
#endif

/* =========================================================
 * 범용 호환 INT 버전
 * - AIX
 * - XL C
 * - GCC
 * - MSVC
 * ========================================================= */
#define LIBCMN_MK_INT(src, len, outVal)         \
    do                                          \
    {                                           \
        int  __copyLen;                         \
        char __buf[MAX_DIGIT + 1];              \
                                                \
        __copyLen = ((len) >= MAX_DIGIT)        \
                    ? MAX_DIGIT                 \
                    : (len);                    \
                                                \
        memcpy(__buf, (src), __copyLen);        \
                                                \
        __buf[__copyLen] = '\0';                \
                                                \
        *(outVal) =                             \
            (int)strtol(__buf, NULL, 10);       \
                                                \
    } while (0)

/* =========================================================
 * 범용 호환 DOUBLE 버전
 * ========================================================= */
#define LIBCMN_MK_DOUBLE(src, len, outVal)      \
    do                                          \
    {                                           \
        int  __copyLen;                         \
        char __buf[MAX_DIGIT + 1];              \
                                                \
        __copyLen = ((len) >= MAX_DIGIT)        \
                    ? MAX_DIGIT                 \
                    : (len);                    \
                                                \
        memcpy(__buf, (src), __copyLen);        \
                                                \
        __buf[__copyLen] = '\0';                \
                                                \
        *(outVal) =                             \
            strtod(__buf, NULL);                \
                                                \
    } while (0)

/* =========================================================
 * GNU GCC 확장 INT 버전
 * - GCC / Clang 전용
 * ========================================================= */
#define GNU_LIBCMN_MK_INT(src, len)             \
    ({                                          \
        int  __copyLen;                         \
        char __buf[MAX_DIGIT + 1];              \
                                                \
        __copyLen = ((len) >= MAX_DIGIT)        \
                    ? MAX_DIGIT                 \
                    : (len);                    \
                                                \
        memcpy(__buf, (src), __copyLen);        \
                                                \
        __buf[__copyLen] = '\0';                \
                                                \
        (int)strtol(__buf, NULL, 10);           \
    })

/* =========================================================
 * GNU GCC 확장 DOUBLE 버전
 * ========================================================= */
#define GNU_LIBCMN_MK_DOUBLE(src, len)          \
    ({                                          \
        int  __copyLen;                         \
        char __buf[MAX_DIGIT + 1];              \
                                                \
        __copyLen = ((len) >= MAX_DIGIT)        \
                    ? MAX_DIGIT                 \
                    : (len);                    \
                                                \
        memcpy(__buf, (src), __copyLen);        \
                                                \
        __buf[__copyLen] = '\0';                \
                                                \
        strtod(__buf, NULL);                    \
    })

int main(void)
{
    int    intValue1 = 0;
    int    intValue2 = 0;

    double doubleValue1 = 0.0;
    double doubleValue2 = 0.0;

    /* -------------------------------------------------
     * INT 예제
     * ------------------------------------------------- */

    /* 범용 버전 */
    LIBCMN_MK_INT("12345", 5, &intValue1);

    /* GNU 반환형 버전 */
    intValue2 = GNU_LIBCMN_MK_INT("67890", 5);

    /* -------------------------------------------------
     * DOUBLE 예제
     * ------------------------------------------------- */

    /* 범용 버전 */
    LIBCMN_MK_DOUBLE("12345.678", 9, &doubleValue1);

    /* GNU 반환형 버전 */
    doubleValue2 = GNU_LIBCMN_MK_DOUBLE("98765.4321", 10);

    /* 출력 */
    printf("intValue1    = %d\n", intValue1);
    printf("intValue2    = %d\n", intValue2);

    printf("doubleValue1 = %f\n", doubleValue1);
    printf("doubleValue2 = %f\n", doubleValue2);

    return 0;
}
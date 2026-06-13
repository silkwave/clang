#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "ase256.c"

/**
 * @brief 데이터를 16진수 형식으로 출력함
 */
void print_hex(const char* label, const uint8_t* buf, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

int main() {
    /* NIST AES-256 테스트 벡터 기반 설정 */
    uint8_t key[32] = {
        0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
        0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
        0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
        0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
    };
    uint8_t iv[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };
    uint8_t round_key[240];

    /* 성능 테스트를 위한 10MB 데이터 할당 */
    const size_t test_size = 10 * 1024 * 1024; // 10MB
    uint8_t* plaintext = (uint8_t*)malloc(test_size);
    uint8_t* data = (uint8_t*)malloc(test_size);

    if (plaintext == NULL || data == NULL) {
        printf("메모리 할당 실패!\n");
        if (plaintext) free(plaintext);
        if (data) free(data);
        return 1;
    }

    /* 평문 데이터 초기화 (패턴 입력) */
    for (size_t i = 0; i < test_size; i++) {
        plaintext[i] = (uint8_t)(i % 256);
    }

    printf("--- AES-256 CBC 성능 벤치마크 (크기: %zu MB) ---\n", test_size / (1024 * 1024));
    
    /* 1. 키 확장 성능 측정 */
    if (KeyExpansion(round_key, key) != 0) {
        printf("키 확장 실패!\n");
        free(plaintext);
        free(data);
        return 1;
    }

    /* 2. 암호화 성능 측정 */
    memcpy(data, plaintext, test_size);
    clock_t start = clock();
    if (AES256_CBC_Encrypt(data, test_size, round_key, iv) != 0) {
        printf("암호화 실패!\n");
    } else {
        clock_t end = clock();
        double enc_time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("암호화 소요 시간: %.4f 초 (속도: %.2f MB/s)\n", enc_time, (test_size / (1024.0 * 1024.0)) / enc_time);
    }

    /* 3. 복호화 성능 측정 */
    start = clock();
    if (AES256_CBC_Decrypt(data, test_size, round_key, iv) != 0) {
        printf("복호화 실패!\n");
    } else {
        clock_t end = clock();
        double dec_time = (double)(end - start) / CLOCKS_PER_SEC;
        printf("복호화 소요 시간: %.4f 초 (속도: %.2f MB/s)\n", dec_time, (test_size / (1024.0 * 1024.0)) / dec_time);
    }

    /* 4. 무결성 검증 (원본 평문과 복호화 결과 비교) */
    if (memcmp(plaintext, data, test_size) == 0) {
        printf("\n결과: 검증 성공 (데이터 무결성 확인됨)\n");
    } else {
        printf("\n결과: 검증 실패 (데이터 오염 감지됨)\n");
    }

    /* 5. 자원 해제 및 보안 소거 */
    free(plaintext);
    free(data);
    secure_memzero(round_key, sizeof(round_key));

    return 0;
}

#include <stdint.h>
#include <string.h>

/* AES-256 규격 상수 정의 */
#define AES256_ROUNDS 14
#define Nb 4  /* 블록 내 워드 수 (4워드 = 16바이트) */
#define Nk 8  /* 키 내 워드 수 (8워드 = 32바이트) */
#define Nr 14 /* 라운드 횟수 */

/* AES S-box (치환 테이블) */
static const uint8_t sbox[256] = {
0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

/* AES 역 S-box (역치환 테이블) */
static const uint8_t rsbox[256] = {
0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
};

/* 라운드 상수 (Round Constant) */
static const uint8_t Rcon[15] = {
0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36,0x6c,0xd8,0xab,0x4d,0x9a
};

/* AES 상태 행렬 타입 (4x4 행렬) */
typedef uint8_t state_t[4][4];

/**
 * @brief 유한체 GF(2^8)에서의 x 2 연산 (xtime)
 */
static uint8_t xtime(uint8_t x) {
    return (x << 1) ^ ((x >> 7) * 0x1b);
}

/* ---------------- 키 확장 (Key Expansion) ---------------- */

/**
 * @brief 컴파일러 최적화에 의해 지워지지 않는 안전한 메모리 소거 함수
 */
static void secure_memzero(void* p, size_t len) {
    if (p == NULL) return;
    volatile uint8_t* vp = (volatile uint8_t*)p;
    while (len--) *vp++ = 0;
}

/**
 * @brief 256비트 키를 사용하여 라운드 키를 생성함
 * @return 성공 시 0, 실패 시 -1
 */
int KeyExpansion(uint8_t* RoundKey, const uint8_t* Key) {
    if (RoundKey == NULL || Key == NULL) return -1;

    uint8_t tempa[4];

    /* 첫 번째 라운드 키는 원본 키와 동일함 */
    for (int i = 0; i < Nk; i++) {
        RoundKey[i*4+0] = Key[i*4+0];
        RoundKey[i*4+1] = Key[i*4+1];
        RoundKey[i*4+2] = Key[i*4+2];
        RoundKey[i*4+3] = Key[i*4+3];
    }

    /* 나머지 라운드 키 생성 */
    for (int i = Nk; i < Nb * (Nr + 1); i++) {
        int k = (i - 1) * 4;
        tempa[0] = RoundKey[k+0];
        tempa[1] = RoundKey[k+1];
        tempa[2] = RoundKey[k+2];
        tempa[3] = RoundKey[k+3];

        if (i % Nk == 0) {
            /* RotWord: 워드 회전 */
            uint8_t t = tempa[0];
            tempa[0] = tempa[1];
            tempa[1] = tempa[2];
            tempa[2] = tempa[3];
            tempa[3] = t;

            /* SubWord: S-box 치환 */
            tempa[0] = sbox[tempa[0]];
            tempa[1] = sbox[tempa[1]];
            tempa[2] = sbox[tempa[2]];
            tempa[3] = sbox[tempa[3]];

            /* Rcon 적용 (인덱스 버그 수정됨) */
            tempa[0] ^= Rcon[(i/Nk) - 1];
        }
        else if (i % Nk == 4) {
            /* AES-256 특화: Nk가 6보다 클 때 추가되는 SubWord */
            tempa[0] = sbox[tempa[0]];
            tempa[1] = sbox[tempa[1]];
            tempa[2] = sbox[tempa[2]];
            tempa[3] = sbox[tempa[3]];
        }

        int j = i * 4;
        int p = (i - Nk) * 4;
        RoundKey[j+0] = RoundKey[p+0] ^ tempa[0];
        RoundKey[j+1] = RoundKey[p+1] ^ tempa[1];
        RoundKey[j+2] = RoundKey[p+2] ^ tempa[2];
        RoundKey[j+3] = RoundKey[p+3] ^ tempa[3];
    }

    /* 임시 버퍼 보안 소거 */
    secure_memzero(tempa, 4);
    return 0;
}

/* ---------------- 암호화 연산 (Cipher Operations) ---------------- */

/**
 * @brief 라운드 키를 상태 행렬에 XOR함
 */
static void AddRoundKey(uint8_t round, state_t* state, const uint8_t* RoundKey) {
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            (*state)[r][c] ^= RoundKey[round * Nb * 4 + c * 4 + r];
}

/**
 * @brief S-box를 사용하여 상태 행렬의 바이트를 치환함
 */
static void SubBytes(state_t* state) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            (*state)[r][c] = sbox[(*state)[r][c]];
}

/**
 * @brief 역 S-box를 사용하여 상태 행렬의 바이트를 역치환함
 */
static void InvSubBytes(state_t* state) {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            (*state)[r][c] = rsbox[(*state)[r][c]];
}

/**
 * @brief 상태 행렬의 행들을 왼쪽으로 순환 이동함
 */
static void ShiftRows(state_t* state) {
    uint8_t temp;
    /* row1: 1바이트 왼쪽 이동 */
    temp = (*state)[1][0];
    (*state)[1][0] = (*state)[1][1];
    (*state)[1][1] = (*state)[1][2];
    (*state)[1][2] = (*state)[1][3];
    (*state)[1][3] = temp;

    /* row2: 2바이트 왼쪽 이동 */
    temp = (*state)[2][0];
    (*state)[2][0] = (*state)[2][2];
    (*state)[2][2] = temp;
    temp = (*state)[2][1];
    (*state)[2][1] = (*state)[2][3];
    (*state)[2][3] = temp;

    /* row3: 3바이트 왼쪽 이동 (=1바이트 오른쪽 이동) */
    temp = (*state)[3][3];
    (*state)[3][3] = (*state)[3][2];
    (*state)[3][2] = (*state)[3][1];
    (*state)[3][1] = (*state)[3][0];
    (*state)[3][0] = temp;
}

/**
 * @brief 상태 행렬의 행들을 오른쪽으로 순환 이동함 (역연산)
 */
static void InvShiftRows(state_t* state) {
    uint8_t temp;
    /* row1: 1바이트 오른쪽 이동 */
    temp = (*state)[1][3];
    (*state)[1][3] = (*state)[1][2];
    (*state)[1][2] = (*state)[1][1];
    (*state)[1][1] = (*state)[1][0];
    (*state)[1][0] = temp;

    /* row2: 2바이트 오른쪽 이동 */
    temp = (*state)[2][0];
    (*state)[2][0] = (*state)[2][2];
    (*state)[2][2] = temp;
    temp = (*state)[2][1];
    (*state)[2][1] = (*state)[2][3];
    (*state)[2][3] = temp;

    /* row3: 3바이트 오른쪽 이동 (=1바이트 왼쪽 이동) */
    temp = (*state)[3][0];
    (*state)[3][0] = (*state)[3][1];
    (*state)[3][1] = (*state)[3][2];
    (*state)[3][2] = (*state)[3][3];
    (*state)[3][3] = temp;
}

/**
 * @brief 상태 행렬의 열들을 혼합함 (최적화 버전)
 */
static void MixColumns(state_t* state) {
    for (int c = 0; c < 4; c++) {
        uint8_t a0 = (*state)[0][c];
        uint8_t a1 = (*state)[1][c];
        uint8_t a2 = (*state)[2][c];
        uint8_t a3 = (*state)[3][c];

        (*state)[0][c] = (uint8_t)(xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3);
        (*state)[1][c] = (uint8_t)(a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3);
        (*state)[2][c] = (uint8_t)(a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3));
        (*state)[3][c] = (uint8_t)((xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3));
    }
}

/**
 * @brief 상태 행렬의 열들을 역혼합함 (성능 최적화 버전)
 */
static void InvMixColumns(state_t* state) {
    for (int c = 0; c < 4; c++) {
        uint8_t a0 = (*state)[0][c];
        uint8_t a1 = (*state)[1][c];
        uint8_t a2 = (*state)[2][c];
        uint8_t a3 = (*state)[3][c];

        /* 유한체 곱셈 최적화 (0x0e, 0x0b, 0x0d, 0x09) */
        uint8_t x2_a0 = xtime(a0);
        uint8_t x4_a0 = xtime(x2_a0);
        uint8_t x8_a0 = xtime(x4_a0);

        uint8_t x2_a1 = xtime(a1);
        uint8_t x4_a1 = xtime(x2_a1);
        uint8_t x8_a1 = xtime(x4_a1);

        uint8_t x2_a2 = xtime(a2);
        uint8_t x4_a2 = xtime(x2_a2);
        uint8_t x8_a2 = xtime(x4_a2);

        uint8_t x2_a3 = xtime(a3);
        uint8_t x4_a3 = xtime(x2_a3);
        uint8_t x8_a3 = xtime(x4_a3);

        (*state)[0][c] = (x8_a0 ^ x4_a0 ^ x2_a0) ^ (x8_a1 ^ x2_a1 ^ a1) ^ (x8_a2 ^ x4_a2 ^ a2) ^ (x8_a3 ^ a3);
        (*state)[1][c] = (x8_a0 ^ a0) ^ (x8_a1 ^ x4_a1 ^ x2_a1) ^ (x8_a2 ^ x2_a2 ^ a2) ^ (x8_a3 ^ x4_a3 ^ a3);
        (*state)[2][c] = (x8_a0 ^ x4_a0 ^ a0) ^ (x8_a1 ^ a1) ^ (x8_a2 ^ x4_a2 ^ x2_a2) ^ (x8_a3 ^ x2_a3 ^ a3);
        (*state)[3][c] = (x8_a0 ^ x2_a0 ^ a0) ^ (x8_a1 ^ x4_a1 ^ a1) ^ (x8_a2 ^ a2) ^ (x8_a3 ^ x4_a3 ^ x2_a3);
    }
}

/* ---------------- 블록 단위 암복호화 (16바이트) ---------------- */

/**
 * @brief 16바이트 블록 하나를 암호화함
 * @return 성공 시 0, 실패 시 -1
 */
int AES256_EncryptBlock(uint8_t* buf, const uint8_t* RoundKey) {
    if (buf == NULL || RoundKey == NULL) return -1;
    state_t* state = (state_t*)buf;

    AddRoundKey(0, state, RoundKey);

    for (int round = 1; round < Nr; round++) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(round, state, RoundKey);
    }

    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(Nr, state, RoundKey);
    return 0;
}

/**
 * @brief 16바이트 블록 하나를 복호화함
 * @return 성공 시 0, 실패 시 -1
 */
int AES256_DecryptBlock(uint8_t* buf, const uint8_t* RoundKey) {
    if (buf == NULL || RoundKey == NULL) return -1;
    state_t* state = (state_t*)buf;

    AddRoundKey(Nr, state, RoundKey);
    InvShiftRows(state);
    InvSubBytes(state);

    for (int round = Nr - 1; round >= 1; round--) {
        AddRoundKey(round, state, RoundKey);
        InvMixColumns(state);
        InvShiftRows(state);
        InvSubBytes(state);
    }

    AddRoundKey(0, state, RoundKey);
    return 0;
}

/* ---------------- CBC 운영 모드 (Cipher Block Chaining) ---------------- */

/**
 * @brief 블록 단위 XOR 연산
 */
static void xor_block(uint8_t* dst, const uint8_t* src) {
    for (int i = 0; i < 16; i++) dst[i] ^= src[i];
}

/**
 * @brief 데이터를 CBC 모드로 암호화함 (데이터 길이는 16의 배수여야 함)
 * @return 성공 시 0, 실패 시 음수
 */
int AES256_CBC_Encrypt(uint8_t* buf, size_t len, const uint8_t* RoundKey, const uint8_t* iv) {
    if (buf == NULL || RoundKey == NULL || iv == NULL) return -1;
    if (len == 0 || len % 16 != 0) return -2;

    uint8_t prev[16];
    memcpy(prev, iv, 16);

    for (size_t off = 0; off < len; off += 16) {
        xor_block(buf + off, prev);
        AES256_EncryptBlock(buf + off, RoundKey);
        memcpy(prev, buf + off, 16);
    }

    secure_memzero(prev, 16);
    return 0;
}

/**
 * @brief 데이터를 CBC 모드로 복호화함 (데이터 길이는 16의 배수여야 함)
 * @return 성공 시 0, 실패 시 음수
 */
int AES256_CBC_Decrypt(uint8_t* buf, size_t len, const uint8_t* RoundKey, const uint8_t* iv) {
    if (buf == NULL || RoundKey == NULL || iv == NULL) return -1;
    if (len == 0 || len % 16 != 0) return -2;

    uint8_t prev[16], cur[16];
    memcpy(prev, iv, 16);

    for (size_t off = 0; off < len; off += 16) {
        memcpy(cur, buf + off, 16);
        AES256_DecryptBlock(buf + off, RoundKey);
        xor_block(buf + off, prev);
        memcpy(prev, cur, 16);
    }

    secure_memzero(prev, 16);
    secure_memzero(cur, 16);
    return 0;
}

/* ---------------- 사용 예시 ----------------
uint8_t key[32] = { ... };          // 256비트 키
uint8_t iv[16]  = { ... };          // 초기화 벡터(IV)
uint8_t RoundKey[240];              // 확장된 라운드 키 버퍼

// 키 확장 수행
if (KeyExpansion(RoundKey, key) != 0) {
    // 에러 처리
}

// CBC 암호화
if (AES256_CBC_Encrypt(data, data_len, RoundKey, iv) != 0) {
    // 에러 처리
}

// CBC 복호화
if (AES256_CBC_Decrypt(data, data_len, RoundKey, iv) != 0) {
    // 에러 처리
}

// 사용 완료 후 민감 정보 안전하게 삭제
secure_memzero(RoundKey, sizeof(RoundKey));
------------------------------------------------- */

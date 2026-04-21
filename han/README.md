# han

`main.c`에는 EUC-KR 전문(바이트 버퍼) 처리용 유틸 함수 2개가 있습니다.

## 함수

- `libcmn_KSCLR(char *buf, int len)`
  - `len` 바이트 기준으로 마지막 한글이 1바이트만 남아 깨질 수 있으면 `buf[len-1]`을 공백으로 치환하고 `buf[len] = '\0'`로 종료합니다.
  - 호출자는 `buf[len]`까지 쓸 수 있게 최소 `len+1` 바이트를 확보해야 합니다.
- `libcmn_KSALPHA(unsigned char *pc_Inbuf, int pi_ILen, unsigned char *rc_Outbuf)`
  - 전각 영숫자(선행 `0xA3`)는 반각 ASCII로, 전각 공백/물결(선행 `0xA1`) 일부는 반각으로 변환합니다.
  - 그 외(한글 포함)는 원형 유지합니다.
  - 출력은 `pi_ILen` 바이트만 채우며, 널 종료는 하지 않습니다(필요하면 호출자가 처리).

## 사용 예제 빌드/실행

`main.c` 하단에 `LIBCMN_EXAMPLE` 가드로 예제 `main()`이 포함되어 있습니다.

### Makefile 사용

```sh
make        # libhan.a + example 빌드
make run    # example 실행
make clean  # 정리
```

### 단일 파일로 컴파일

```sh
cc -DLIBCMN_EXAMPLE -o example main.c
./example
```

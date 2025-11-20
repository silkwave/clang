#include <stdio.h>
#include <stdlib.h>

int main() {
    int *p = malloc(100 * sizeof(int)); // 누수
    int *x = malloc(100 * sizeof(int)); // 누수    

    free(p);
    free(x);

    printf("Program exit.\n");
    return 0;
}

/*
valgrind 사용  
gcc -g main.c -o app
valgrind --leak-check=full ./app
valgrind --leak-check=full ./ substr_test.exe

ASan으로 실행
gcc -fsanitize=address -fno-omit-frame-pointer -g main.c -o app
./app


*/
/*
 * 이 파일은 C 언어에서 발생할 수 있는 일반적인 메모리 관련 오류(메모리 누수, 버퍼 오버런, 이중 해제, 해제 후 사용)를
 * 시연하기 위해 작성된 예제 코드입니다.
 * 각 함수는 특정 유형의 메모리 오류를 의도적으로 포함하고 있습니다.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

Node* create_node(const char *text) {
    // 목적: 새 노드를 생성하고 주어진 텍스트로 초기화합니다.
    // 문제점: 이 함수 자체는 메모리를 해제하지 않습니다.
    //         호출자가 반환된 Node*와 그 내부의 node->data를 반드시 free()해야 합니다.
    //         그렇지 않으면 메모리 누수가 발생합니다.
    Node *node = (Node*)malloc(sizeof(Node));
    if (node == NULL) { // malloc 실패 처리
        perror("Failed to allocate Node");
        exit(EXIT_FAILURE);
    }
    node->data = strdup(text);     // strdup가 내부적으로 malloc 사용
    if (node->data == NULL) { // strdup 실패 처리
        perror("Failed to duplicate string");
        free(node);
        exit(EXIT_FAILURE);
    }
    node->next = NULL;
    return node;
}

void buggy_buffer_write() {
    // 목적: 버퍼 오버런(Buffer Overrun)을 시연합니다.
    // 문제점: 10바이트만 할당된 'buf'에 20바이트를 쓰려고 시도합니다.
    //         이는 할당된 메모리 경계를 넘어 다른 메모리 영역을 덮어쓰게 됩니다.
    char *buf = malloc(10);
    if (buf == NULL) { // malloc 실패 처리
        perror("Failed to allocate buffer");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 20; i++) {
        buf[i] = 'A';              // 10바이트 malloc → 20바이트 write = 버퍼 오버런
    }
    free(buf); // 할당된 메모리 해제 (오버런은 발생하지만, 이 함수 내에서 할당된 메모리는 해제)
}

void double_free_example() {
    // 목적: 이중 해제(Double Free) 오류를 시연합니다.
    // 문제점: 'p'가 가리키는 메모리 블록을 두 번 해제하려고 시도합니다.
    //         이는 힙 손상(heap corruption)을 일으키거나 프로그램 충돌로 이어질 수 있습니다.
    char *p = malloc(50);
    if (p == NULL) { // malloc 실패 처리
        perror("Failed to allocate memory for double free example");
        exit(EXIT_FAILURE);
    }
    free(p);
    free(p);                       // 더블 free
}

void use_after_free_example() {
    // 목적: 해제 후 사용(Use-After-Free, UAF) 오류를 시연합니다.
    // 문제점: 'p'가 가리키는 메모리를 해제한 후, 해당 메모리 위치에 접근하여 값을 쓰려고 시도합니다.
    //         이는 예측 불가능한 동작, 데이터 손상 또는 보안 취약점으로 이어질 수 있습니다.
    char *p = malloc(20);
    if (p == NULL) { // malloc 실패 처리
        perror("Failed to allocate memory for use-after-free example");
        exit(EXIT_FAILURE);
    }
    free(p);
    p[0] = 'X';                    // free 후 write → UAF
}

Node* create_list() {
    // 목적: 연결 리스트를 생성하고 메모리 누수(Memory Leak)를 시연합니다.
    // 문제점: 'create_node'를 통해 할당된 노드들과 그 내부의 문자열 데이터가
    //         이 함수 내에서 또는 호출자(main 함수)에서 해제되지 않습니다.
    //         결과적으로 이 함수가 반환된 후 할당된 메모리는 회수되지 않고 누수됩니다.
    Node *head = create_node("first");
    head->next = create_node("second");
    head->next->next = create_node("third");
    return head;                   // 전체 리스트 free 안함 → 누수
}

int main() {
    printf("Running complex memory test...\n");

    // 1. 메모리 누수 시연: create_list 함수에서 할당된 노드들이 해제되지 않아 메모리 누수가 발생합니다.
    Node *list = create_list();
    // 참고: 실제 애플리케이션에서는 list를 순회하며 모든 노드와 데이터를 free()해야 합니다.
    // 이 예제에서는 의도적으로 누수를 발생시킵니다.

    // 2. 버퍼 오버런 시연: buggy_buffer_write 함수 내에서 할당된 버퍼의 경계를 넘어 데이터를 씁니다.
    buggy_buffer_write();

    // 3. 이중 해제 시연: double_free_example 함수 내에서 동일한 메모리 블록을 두 번 해제합니다.
    double_free_example();

    // 4. 해제 후 사용 시연: use_after_free_example 함수 내에서 해제된 메모리에 접근하여 데이터를 씁니다.
    use_after_free_example();

    printf("Program exiting...\n");
    return 0;
}

/*

gcc -fsanitize=address -fno-omit-frame-pointer -g memory_test.c -o test
./test

gcc -g memory_test.c -o test
valgrind --leak-check=full --track-origins=yes ./test


*/
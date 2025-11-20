#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

Node* create_node(const char *text) {
    Node *node = (Node*)malloc(sizeof(Node));
    node->data = strdup(text);     // strdup가 내부적으로 malloc 사용
    node->next = NULL;
    return node;                   // free 안함 → 누수
}

void buggy_buffer_write() {
    char *buf = malloc(10);
    for (int i = 0; i < 20; i++) {
        buf[i] = 'A';              // 10바이트 malloc → 20바이트 write = 버퍼 오버런
    }
}

void double_free_example() {
    char *p = malloc(50);
    free(p);
    free(p);                       // 더블 free
}

void use_after_free_example() {
    char *p = malloc(20);
    free(p);
    p[0] = 'X';                    // free 후 write → UAF
}

Node* create_list() {
    Node *head = create_node("first");
    head->next = create_node("second");
    head->next->next = create_node("third");
    return head;                   // 전체 리스트 free 안함 → 누수
}

int main() {
    printf("Running complex memory test...\n");

    // 1. 복잡한 구조체 누수
    Node *list = create_list();

    // 2. 버퍼 오버런
    buggy_buffer_write();

    // 3. 더블 free
    double_free_example();

    // 4. 해제 후 사용
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
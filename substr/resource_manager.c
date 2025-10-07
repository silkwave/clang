/*
 * resource_manager.c
 * - 단순 리소스(메모리) 등록/해제 관리 구현
 *
 * 동작:
 * - `register_resource`로 등록된 포인터는 내부 배열에 저장되고, 프로그램 종료 시 atexit 훅으로 전부 해제됩니다.
 * - `unregister_resource`는 특정 포인터를 찾아 free 하고 목록에서 제거합니다.
 * - 스레드 안전을 위해 mutex를 사용합니다.
 */

#include "resource_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define MAX_RESOURCES 1024

static void* resources[MAX_RESOURCES];
static int resource_count_ = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int initialized = 0;

/* 모든 등록된 리소스를 해제하고 카운트를 초기화합니다. atexit로 호출됩니다. */
static void cleanup_all(void) {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < resource_count_; i++) {
        free(resources[i]);
        resources[i] = NULL;
    }
    resource_count_ = 0;
    pthread_mutex_unlock(&lock);
    printf("[ResourceManager] 모든 리소스 해제 완료\n");
}

void register_resource(void* ptr) {
    if (!ptr) return;
    pthread_mutex_lock(&lock);
    if (!initialized) {
        /* 프로그램 종료 시 cleanup_all이 호출되도록 등록 */
        atexit(cleanup_all);
        initialized = 1;
    }
    if (resource_count_ < MAX_RESOURCES)
        resources[resource_count_++] = ptr;
    else
        fprintf(stderr, "[ResourceManager] 등록 한도 초과\n");
    pthread_mutex_unlock(&lock);
}

/* 등록된 포인터를 찾아 free하고 목록에서 제거합니다. */
void unregister_resource(void* ptr) {
    if (!ptr) return;
    pthread_mutex_lock(&lock);
    for (int i = 0; i < resource_count_; i++) {
        if (resources[i] == ptr) {
            free(resources[i]);
            for (int j = i; j < resource_count_ - 1; j++)
                resources[j] = resources[j + 1];
            resource_count_--;
            break;
        }
    }
    pthread_mutex_unlock(&lock);
}

/* realloc 후 내부 배열에서 포인터를 갱신합니다. */
void* realloc_resource(void* old_ptr, size_t new_size) {
    if (!old_ptr || new_size == 0) return NULL;
    pthread_mutex_lock(&lock);
    void* new_ptr = realloc(old_ptr, new_size);
    if (!new_ptr) {
        pthread_mutex_unlock(&lock);
        return NULL;
    }
    for (int i = 0; i < resource_count_; i++) {
        if (resources[i] == old_ptr) {
            resources[i] = new_ptr;
            break;
        }
    }
    pthread_mutex_unlock(&lock);
    return new_ptr;
}

void cleanup_resources(void) {
    cleanup_all();
}

int resource_count(void) {
    pthread_mutex_lock(&lock);
    int count = resource_count_;
    pthread_mutex_unlock(&lock);
    return count;
}

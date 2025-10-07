#include "resource_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define MAX_RESOURCES 1024

static void* resources[MAX_RESOURCES];
static int resource_count_ = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int initialized = 0;

/// 내부 전용: 전체 해제
static void cleanup_all(void) {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < resource_count_; i++) {
        void* target = resources[i];
        printf("[ResourceManager] free: %p\n", target);
        free(target);
        resources[i] = NULL;
    }
    resource_count_ = 0;
    pthread_mutex_unlock(&lock);
    printf("[ResourceManager] 모든 리소스 해제 완료\n");
}

/// 자원 등록
void register_resource(void* ptr) {
    if (!ptr) return;

    pthread_mutex_lock(&lock);

    if (!initialized) {
        atexit(cleanup_all);
        initialized = 1;
    }

    if (resource_count_ < MAX_RESOURCES) {
        resources[resource_count_++] = ptr;
        printf("[ResourceManager] 등록: %p (총 %d개)\n", ptr, resource_count_);
    } else {
        fprintf(stderr, "[ResourceManager] 등록 한도 초과\n");
    }

    pthread_mutex_unlock(&lock);
}

/// 특정 자원 해제 및 목록에서 제거
void unregister_resource(void* ptr) {
    if (!ptr) return;

    pthread_mutex_lock(&lock);
    for (int i = 0; i < resource_count_; i++) {
        if (resources[i] == ptr) {
            void* addr = resources[i]; // 로그용 사본

            printf("[ResourceManager] 해제 시도: %p\n", addr);
            free(resources[i]);        // 메모리 해제
            resources[i] = NULL;

            for (int j = i; j < resource_count_ - 1; j++) {
                resources[j] = resources[j + 1];
            }
            resource_count_--;

            // free 이후 로그 시 원본 포인터 사용 금지 → 단순 카운트만 표시
            printf("[ResourceManager] 해제 완료 (남은 %d개)\n", resource_count_);
            break;
        }
    }
    pthread_mutex_unlock(&lock);
}

/// realloc 후 포인터 갱신
void* realloc_resource(void* old_ptr, size_t new_size) {
    if (!old_ptr || new_size == 0) return NULL;

    pthread_mutex_lock(&lock);

    // realloc 전에 로그 출력
    printf("[ResourceManager] realloc 시도: %p → (size=%zu)\n", old_ptr, new_size);

    void* new_ptr = realloc(old_ptr, new_size);
    if (!new_ptr) {
        pthread_mutex_unlock(&lock);
        return NULL;
    }

    // 목록 내 포인터 갱신
    for (int i = 0; i < resource_count_; i++) {
        if (resources[i] == old_ptr) {
            resources[i] = new_ptr;
            break;
        }
    }

    // realloc 후에 새 포인터만 출력 (old_ptr 언급 금지)
    printf("[ResourceManager] realloc 완료: 새 포인터 %p\n", new_ptr);

    pthread_mutex_unlock(&lock);
    return new_ptr;
}

/// 등록된 자원 전체 해제 (수동 호출용)
void cleanup_resources(void) {
    cleanup_all();
}

/// 현재 등록된 자원 개수
int resource_count(void) {
    pthread_mutex_lock(&lock);
    int count = resource_count_;
    pthread_mutex_unlock(&lock);
    return count;
}

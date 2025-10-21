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

static void **resources = NULL;
static int resource_count_ = 0;
static int resource_capacity_ = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int initialized = 0;

/* 모든 등록된 리소스를 해제하고 카운트를 초기화합니다. atexit로 호출됩니다. */
static void cleanup_all(void)
{
    pthread_mutex_lock(&lock);
    if (resource_count_ == 0 && resources == NULL)
    {
        pthread_mutex_unlock(&lock);
        return; // 이미 정리되었으면 아무것도 하지 않음
    }
    for (int i = 0; i < resource_count_; i++)
    {
        free(resources[i]);
    }
    free(resources);
    resources = NULL;
    resource_count_ = resource_capacity_ = 0;
    pthread_mutex_unlock(&lock);
    fprintf(stderr, "[ResourceManager] 모든 리소스 해제 완료\n");
}

void register_resource(void *ptr)
{
    if (!ptr)
        return;
    pthread_mutex_lock(&lock);
    if (!initialized)
    {
        /* 프로그램 종료 시 cleanup_all이 호출되도록 등록 */
        atexit(cleanup_all);
        initialized = 1;
    }

    if (resource_count_ >= resource_capacity_)
    {
        int new_capacity = (resource_capacity_ == 0) ? 16 : resource_capacity_ * 2;
        fprintf(stderr, "[ResourceManager] 리소스 배열 확장: %d -> %d\n", resource_capacity_, new_capacity);
        void **new_resources = realloc(resources, new_capacity * sizeof(void *));
        if (!new_resources)
        {
            fprintf(stderr, "[ResourceManager] 리소스 배열 확장 실패. 메모리 누수 발생 가능.\n");
            free(ptr); // 등록 실패 시 할당된 메모리 해제
            pthread_mutex_unlock(&lock);
            return;
        }
        resources = new_resources;
        resource_capacity_ = new_capacity;
    }
    resources[resource_count_++] = ptr;
    fprintf(stderr, "[ResourceManager] 리소스 등록: %p (현재 %d개)\n", ptr, resource_count_);
    pthread_mutex_unlock(&lock);
}

/* 등록된 포인터를 찾아 free하고 목록에서 제거합니다. */
void unregister_resource(void *ptr)
{
    if (!ptr)
        return;
    pthread_mutex_lock(&lock);
    int found = 0;
    for (int i = 0; i < resource_count_; i++)
    {
        if (resources[i] == ptr)
        {
            free(resources[i]);
            resource_count_--;
            resources[i] = resources[resource_count_];
            fprintf(stderr, "[ResourceManager] 리소스 해제: %p (남은 개수 %d개)\n", ptr, resource_count_);
            found = 1;
            break;
        }
    }
    if (!found)
        fprintf(stderr, "[ResourceManager] 해제할 리소스를 찾을 수 없음: %p\n", ptr);
    pthread_mutex_unlock(&lock);
}

/* realloc 후 내부 배열에서 포인터를 갱신합니다. */
void *realloc_resource(void *old_ptr, size_t new_size)
{
    if (!old_ptr || new_size == 0)
    {
        // new_size가 0이면 free와 동일하게 동작하도록 unregister 호출
        if (old_ptr && new_size == 0)
        {
            unregister_resource(old_ptr);
        }
        return NULL;
    }

    pthread_mutex_lock(&lock);

    int found_idx = -1;
    for (int i = 0; i < resource_count_; i++)
    {
        if (resources[i] == old_ptr)
        {
            found_idx = i;
            break;
        }
    }

    if (found_idx != -1)
    {
        void *new_ptr = realloc(old_ptr, new_size);
        if (new_ptr)
        {
            resources[found_idx] = new_ptr;
            fprintf(stderr, "[ResourceManager] 리소스 재할당 성공: %p -> %p (새 크기: %zu)\n", old_ptr, new_ptr, new_size);
        }
        else
        {
            fprintf(stderr, "[ResourceManager] 리소스 재할당 실패: %p (크기: %zu)\n", old_ptr, new_size);
        }
        pthread_mutex_unlock(&lock);
        return new_ptr;
    }

    pthread_mutex_unlock(&lock);
    // 관리되지 않는 포인터에 대한 realloc 시도.
    fprintf(stderr, "[ResourceManager] 재할당할 리소스를 찾을 수 없음: %p\n", old_ptr);
    return NULL;
}

/* atexit 핸들러를 직접 호출하기 위한 래퍼 함수 */
void cleanup_resources(void)
{
    cleanup_all();
}

int resource_count(void)
{
    pthread_mutex_lock(&lock);
    int count = resource_count_;
    pthread_mutex_unlock(&lock);
    return count;
}

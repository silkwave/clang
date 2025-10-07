/*
 * resource_manager.h
 * - 동적 할당된 리소스(포인터)를 전역으로 등록/해제하여 프로그램 종료 시 자동 정리하는 유틸리티
 *
 * 사용 방법:
 * - 동적으로 할당한 메모리 포인터를 `register_resource`로 등록하면, atexit 훅으로 자동 해제됩니다.
 * - 개별 해제가 필요하면 `unregister_resource`를 호출하면 등록 해제와 함께 free 합니다.
 */

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <stddef.h>

void register_resource(void* ptr);
void unregister_resource(void* ptr);
void* realloc_resource(void* old_ptr, size_t new_size);
void cleanup_resources(void);
int resource_count(void);

#endif // RESOURCE_MANAGER_H

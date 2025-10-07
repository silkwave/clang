#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <stddef.h>

/// malloc() 또는 realloc()으로 확보된 메모리를 등록
void register_resource(void* ptr);

/// 특정 포인터를 해제 및 목록에서 제거
void unregister_resource(void* ptr);

/// 등록된 포인터의 크기를 변경 (realloc)
/// 반환된 포인터는 교체되어 목록에 자동 갱신됨
void* realloc_resource(void* old_ptr, size_t new_size);

/// 모든 자원 해제 (프로그램 종료 시 자동 호출)
void cleanup_resources(void);

/// 현재 등록된 자원 개수 조회
int resource_count(void);

#endif // RESOURCE_MANAGER_H

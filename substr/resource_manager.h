#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <stddef.h>

void register_resource(void* ptr);
void unregister_resource(void* ptr);
void* realloc_resource(void* old_ptr, size_t new_size);
void cleanup_resources(void);
int resource_count(void);

#endif // RESOURCE_MANAGER_H

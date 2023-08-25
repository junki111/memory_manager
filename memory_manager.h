#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stddef.h>

typedef struct MemoryBlock {
    size_t size;
    struct MemoryBlock *next;
} MemoryBlock;

typedef struct MemoryManager {
    void *memoryPool;
    size_t size;
    MemoryBlock *freeBlocks; // Linked list of free blocks
} MemoryManager;

int initializeMemoryManager(MemoryManager *memoryManager, size_t poolSize);
void *allocationMemory(MemoryManager *memoryManager, size_t size);
void deallocateMemory(MemoryManager *memoryManager, void *ptr, void **allocateAddresses, size_t *numAllocatedAddresses);

#endif // MEMORY_MANAGER_H

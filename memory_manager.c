#include <stdio.h>
#include <stdlib.h>
#include "memory_manager.h"

int initializeMemoryManager(MemoryManager *manager, size_t poolSize) {
    manager->memoryPool = malloc(poolSize);
    if (manager->memoryPool == NULL) {
        printf("FAILURE: Memory initialization failed.\n");
        return 0;
    } else {
        manager->size = poolSize;

        // Initialize the initial free block
        manager->freeBlocks = (MemoryBlock *) manager->memoryPool;
        manager->freeBlocks->size = poolSize;
        manager->freeBlocks->next = NULL;

        printf("SUCCESS\n");
        return 1;
    }
}

void *allocationMemory(MemoryManager *manager, size_t size) {
    if (manager->size < size + sizeof(MemoryBlock)) {
        printf("FAILURE: Not enough memory available.\n");
        return NULL;
    }

    // Allocate a new block from the pool
    MemoryBlock *currentBlock = manager->freeBlocks;

    while (currentBlock) {
        if (currentBlock->size >= size + sizeof(MemoryBlock)) {
            break;
        }
        currentBlock = currentBlock->next;
    }

    if (!currentBlock) {
        printf("FAILURE: No suitable block found.\n");
        return NULL;
    }

    // Split the block if it's larger than the requested size
    if (currentBlock->size > size + sizeof(MemoryBlock)) {
        MemoryBlock *newBlock = (MemoryBlock *)((char *)currentBlock + size + sizeof(MemoryBlock));
        newBlock->size = currentBlock->size - size - sizeof(MemoryBlock);
        newBlock->next = currentBlock->next;

        currentBlock->size = size;
        currentBlock->next = newBlock;
    }

    // Update the total memory in the pool
    manager->size -= currentBlock->size + sizeof(MemoryBlock);

    printf("SUCCESS\n");
    return (void *)(currentBlock + 1); // Return the address of the memory block after the header(metadata)
}

void deallocateMemory(MemoryManager *manager, void *ptr, void **allocateAddresses, size_t *numAllocatedAddresses) {
    if (ptr == NULL) {
        printf("FAILURE: Attempt to free a NULL pointer\n");
        return;
    }

    MemoryBlock *blockToDeallocate = (MemoryBlock *)ptr - 1;

    // Update the total memory in the pool
    manager->size += blockToDeallocate->size + sizeof(MemoryBlock);

    // Remove the deallocated address from the allocateAddresses array
    for (size_t i = 0; i < *numAllocatedAddresses; i++) {
        if (allocateAddresses[i] == ptr) {
            // Shift the remaining elements to fill the gap
            for (size_t j = i; j < *numAllocatedAddresses - 1; j++) {
                allocateAddresses[j] = allocateAddresses[j + 1];
            }
            (*numAllocatedAddresses)--;
            break;
        }
    }

    // Find the block to deallocate in the free list
    MemoryBlock *currentBlock = manager->freeBlocks;
    MemoryBlock *previousBlock = NULL;

    while (currentBlock) {
        if (currentBlock == blockToDeallocate) {
            break;
        }
        previousBlock = currentBlock;
        currentBlock = currentBlock->next;
    }

    if (currentBlock) {
        // Remove the block from the free list
        if (previousBlock) {
            previousBlock->next = currentBlock->next;
        } else {
            manager->freeBlocks = currentBlock->next;
        }
    } else {
        printf("FAILURE: Block to deallocate not found in the free list\n");
    }

    printf("SUCCESS\n");
}


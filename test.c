#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winbase.h>

#define POOL_SIZE 1024 * 1024 * 1024// Memory pool size in bytes

#define MIN_ALLOCATION_SIZE 1024 *1024 // Minimum allocation size in bytes
#define MAX_ALLOCATION_SIZE 8 * 1024 * 1024 // Maximum allocation size in bytes

#define MIN_NUM_ALLOC_DEALLOC 50 // Minimum number of allocations and deallocations
#define MAX_NUM_ALLOC_DEALLOC 100 // Maximum number of allocations and deallocations
#define FIXED_ALLOCATION_SIZE 2 * 1024 * 1024 // Fixed allocation size in bytes

#define IS_DYNAMIC_ALLOCATION 1 // Set this to 1 to test dynamic allocation, set this to 0 to test fixed allocation

#define MAX_ALLOCATED_ADDRESSES MAX_NUM_ALLOC_DEALLOC
void *allocateAddresses[MAX_ALLOCATED_ADDRESSES]; // Array of allocated addresses
size_t numAllocatedAddresses = 0; // Number of allocated addresses

// Get the current time in microseconds
ULONGLONG getCurrentTimeMicroseconds() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER timestamp;
    timestamp.LowPart = ft.dwLowDateTime;
    timestamp.HighPart = ft.dwHighDateTime;

    return timestamp.QuadPart / 10; // Convert 100 nanoseconds to microsecond
}

int main() {
    srand(time(NULL)); // Seed the random number generator

    // Pool size as defined above
    size_t poolSize = POOL_SIZE;
    printf("Memory pool size: %lu bytes\n", poolSize);
    size_t numAllocDealloc = rand() % (MAX_NUM_ALLOC_DEALLOC - MIN_NUM_ALLOC_DEALLOC + 1) + MIN_NUM_ALLOC_DEALLOC;
    printf("Number of allocations and deallocations: %lu\n", numAllocDealloc);

    // printf("Memory pool size: %lu bytes\n", poolSize);

    // printf("Number of allocations and deallocations: %lu\n", numAllocDealloc);

    MemoryManager manager;

    if (!initializeMemoryManager(&manager, poolSize)) {
        printf("Memory manager initialization failed\n");
        return 0;
    }

    // Metrics to be calculated
    size_t totalSuccessfulAllocations = 0;
    size_t totalSuccessfulDeallocations = 0;
    size_t totalFailedAllocations = 0;
    size_t totalFailedDeallocations = 0;
    long long totalAllocationTime = 0;
    long long totalDeallocationTime = 0;
    size_t totalMemoryAllocated = 0;
    size_t minimumMemoryAllocated = SIZE_MAX;
    size_t maximumMemoryAllocated = 0;

    int isDynamicOrFixedAllocation = IS_DYNAMIC_ALLOCATION % 2; // Randomly choose between dynamic and fixed allocation

    if (isDynamicOrFixedAllocation) { // Dynamic allocation

        for (int i = 0; i < numAllocDealloc; ++i) {
            int isAllocation = rand() % 2; // Randomly choose between allocation and deallocation

            if (isAllocation) { // Dynamic allocation
                size_t allocationSize = rand() % (MAX_ALLOCATION_SIZE - MIN_ALLOCATION_SIZE + 1) + MIN_ALLOCATION_SIZE;

                ULONGLONG startTime = getCurrentTimeMicroseconds();
                void *allocatedBlock = allocationMemory(&manager, allocationSize);
                ULONGLONG endTime = getCurrentTimeMicroseconds();

                if (allocatedBlock) {
                    allocateAddresses[numAllocatedAddresses++] = allocatedBlock; // Add the allocated address to the array of allocated addresses

                    // Calculate allocation time
                    ULONGLONG allocationTime = endTime - startTime;
                    totalAllocationTime += allocationTime;
                    totalSuccessfulAllocations++;
                    totalMemoryAllocated += allocationSize;

                    if (allocationSize > maximumMemoryAllocated) {
                        maximumMemoryAllocated = allocationSize;
                    }

                    if (allocationSize < minimumMemoryAllocated) {
                        minimumMemoryAllocated = allocationSize;
                    }

                } else {
                    totalFailedAllocations++;
                }
            } else { // Dynamic deallocation
                if (totalSuccessfulAllocations > totalSuccessfulDeallocations) { // Make sure that you are not deallocating more than you have allocated
                    int indexToDeallocate = rand() % numAllocatedAddresses;
                    if (indexToDeallocate < numAllocatedAddresses && allocateAddresses[indexToDeallocate] != NULL) {
                        void *addressToDeallocate = allocateAddresses[indexToDeallocate];// Allocate a random address to deallocate
                        // allocateAddresses[indexToDeallocate] = NULL; // Mark the address as deallocated

                        // Remove the deallocated address from the list
                        for (size_t i = indexToDeallocate; i < numAllocatedAddresses - 1; i++) {
                            allocateAddresses[i] = allocateAddresses[i + 1];
                        }
                        numAllocatedAddresses--;

                        // Calculate deallocation time
                        ULONGLONG startTime = getCurrentTimeMicroseconds();
                        deallocateMemory(&manager, addressToDeallocate, allocateAddresses, &numAllocatedAddresses);
                        ULONGLONG endTime = getCurrentTimeMicroseconds();

                        ULONGLONG deallocationTime = endTime - startTime;
                        totalDeallocationTime += deallocationTime;
                        totalSuccessfulDeallocations++;

                    } else {
                        totalFailedDeallocations++;
                        printf("FAILURE\n");
                    }  
                } else {
                    totalFailedDeallocations++;
                    printf("FAILURE\n");
                }
            }
        }
        printf("Dynamic allocation test complete\n");
    } else { // Fixed allocation        
        for (int i = 0; i < numAllocDealloc; ++i) {
            int isAllocation = rand() % 2; // Randomly choose between allocation and deallocation

            if (isAllocation) { // Fixed allocation
                size_t allocationSize = FIXED_ALLOCATION_SIZE;

                // Calculate allocation time
                ULONGLONG startTime = getCurrentTimeMicroseconds();
                void *allocatedBlock = allocationMemory(&manager, allocationSize);
                ULONGLONG endTime = getCurrentTimeMicroseconds();

                if (allocatedBlock) {
                    allocateAddresses[numAllocatedAddresses++] = allocatedBlock; // Add the allocated address to the array of allocated addresses
                    ULONGLONG allocationTime = endTime - startTime;
                    totalAllocationTime += allocationTime;
                    totalSuccessfulAllocations++;
                    totalMemoryAllocated += allocationSize;

                    if (allocationSize > maximumMemoryAllocated) {
                        maximumMemoryAllocated = allocationSize;
                    }

                    if (allocationSize < minimumMemoryAllocated) {
                        minimumMemoryAllocated = allocationSize;
                    }

                } else {
                    totalFailedAllocations++;
                    printf("FAILURE\n");
                }
            } else { // Fixed deallocation
                if (totalSuccessfulAllocations > totalSuccessfulDeallocations) { // Make sure that you are not deallocating more than you have allocated
                    int indexToDeallocate = rand() % numAllocatedAddresses;
                    if (indexToDeallocate < numAllocatedAddresses && allocateAddresses[indexToDeallocate] != NULL) {
                        void *addressToDeallocate = allocateAddresses[indexToDeallocate];// Allocate a random address to deallocate
                        // allocateAddresses[indexToDeallocate] = NULL; // Mark the address as deallocated

                        // Remove the deallocated address from the list
                        for (size_t i = indexToDeallocate; i < numAllocatedAddresses - 1; i++) {
                            allocateAddresses[i] = allocateAddresses[i + 1];
                        }
                        numAllocatedAddresses--;

                        // Calculate deallocation time
                        ULONGLONG startTime = getCurrentTimeMicroseconds();
                        deallocateMemory(&manager, addressToDeallocate, allocateAddresses, &numAllocatedAddresses);
                        ULONGLONG endTime = getCurrentTimeMicroseconds();

                        ULONGLONG deallocationTime = endTime - startTime;
                        totalDeallocationTime += deallocationTime;
                        totalSuccessfulDeallocations++;

                    } else {
                        totalFailedDeallocations++;
                        printf("FAILURE\n");
                    } 
                } else {
                    totalFailedDeallocations++;
                    printf("FAILURE\n");
                }
            }
        }
        printf("Fixed allocation test complete\n");
    }

    printf("Complete");
    // Calculate averages
    double averageAllocationTime = (double)totalAllocationTime / totalSuccessfulAllocations;
    double averageDeallocationTime = (double)totalDeallocationTime / totalSuccessfulDeallocations;
    double averageMemoryAllocated = (double)totalMemoryAllocated / totalSuccessfulAllocations;

    // Print metrics
    printf("Memory pool size: %lu bytes\n", poolSize);
    printf("Number of allocations: %lu\n", totalSuccessfulAllocations);
    printf("Number of deallocations: %lu\n", totalSuccessfulDeallocations);
    printf("Number of failed allocations: %lu\n", totalFailedAllocations);
    printf("Number of failed deallocations: %lu\n", totalFailedDeallocations);
    printf("Average allocation time: %f microseconds\n", averageAllocationTime);
    printf("Average deallocation time: %f microseconds\n", averageDeallocationTime);
    printf("Average memory allocated: %f bytes\n", averageMemoryAllocated);
    printf("Minimum memory allocated: %lu bytes\n", minimumMemoryAllocated);
    printf("Maximum memory allocated: %lu bytes\n", maximumMemoryAllocated);

    free(manager.memoryPool); // Free the memory pool that has been created once the program is done
    return 0;
}

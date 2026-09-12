#include "memory.h"
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct header {
    size_t size;
    struct header * prev;
    struct header * next;
    int in_use;
} m_header;

m_header* freelist = NULL;

void print_freelist() {

    printf("\n --- FREELIST START ---\n");
    m_header* curr = freelist;
    while(curr != NULL) {
        printf("[%p: size:%lu prev:%p next:%p use:%d]\n", curr, curr->size, curr->prev, curr->next, curr->in_use);
        curr = curr->next;
    }
    printf(" --- FREELIST END ---\n\n");
}

void * new_malloc(size_t size) {

    printf("Malloc start.\n");
    if(freelist == NULL) {
        printf("MMAP\n");
         // Use mmap to get anonymous, private memory
        freelist = mmap(NULL,                      // Desired start address (NULL lets OS choose)
                      2048,                        // Length of the mapping (rounded up to page size)
                      PROT_READ | PROT_WRITE,      // Memory protection: readable and writable
                      MAP_PRIVATE | MAP_ANONYMOUS, // Visibility: private to the process, not file-backed
                      -1,                          // File descriptor: -1 for anonymous mapping
                      0);                          // Offset: 0 for anonymous mapping
        
        print_freelist();

        if (freelist == MAP_FAILED) {
            printf("map failed\n");
            freelist = NULL;
            return NULL;
        } else {
            printf("create map\n");
            freelist->size = 2048 - sizeof(m_header);
            freelist->prev = NULL;
            freelist->next = NULL;
            freelist->in_use = 0;
            print_freelist();
        }
    }

    m_header* curr = freelist;
    while(curr != NULL) {
        printf("\tChecking %p: size:%lu prev:%p next:%p use:%d\n", curr, curr->size, curr->prev, curr->next, curr->in_use);
        if (curr->size >= size && curr->in_use == 0) { // pick first one that is big enough
            curr->in_use = 1;
            if (curr-> size != size) {
                // Break into smaller chunks

                int newSize = curr->size - size - sizeof(m_header);

                curr-> size = size;

                m_header* newHeader = malloc(sizeof(m_header));
                newHeader->size = newSize;
                newHeader->in_use = 0;
                newHeader->prev = curr;
                newHeader->next = curr->next;
                curr->next = newHeader;
            }

            print_freelist();
            printf("Malloc return\n");
            return curr;
        }
        curr = curr->next;
    }
    printf("Malloc failed.  Returning NULL\n");
    return NULL;
}

void new_free(void * ptr) {

    printf("Free start.\n");
    //print_freelist();

    m_header* headerPtr = (m_header*)ptr;
    printf("\tFreeing %p: size:%lu prev:%p next:%p use:%d\n", headerPtr, headerPtr->size, headerPtr->prev, headerPtr->next, headerPtr->in_use);
    ((m_header*)ptr)->in_use = 0;
    // print_freelist();

    printf("Free end.\n");
}
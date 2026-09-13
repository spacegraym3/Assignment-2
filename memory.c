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

    printf(" - FREELIST\n");
    m_header* curr = freelist;
    while(curr != NULL) {
        printf(" - [%p: size:%lu prev:%p next:%p use:%d]\n", curr, curr->size, curr->prev, curr->next, curr->in_use);
        curr = curr->next;
    }
    printf(" -\n");
}

void * new_malloc(size_t size) {

    printf("\n--> malloc size=%lu\n", size);
    if(freelist == NULL) {
        printf("--> malloc MMAP\n");
         // Use mmap to get anonymous, private memory
        freelist = mmap(NULL,                      // Desired start address (NULL lets OS choose)
                      2048,                        // Length of the mapping (rounded up to page size)
                      PROT_READ | PROT_WRITE,      // Memory protection: readable and writable
                      MAP_PRIVATE | MAP_ANONYMOUS, // Visibility: private to the process, not file-backed
                      -1,                          // File descriptor: -1 for anonymous mapping
                      0);                          // Offset: 0 for anonymous mapping

        if (freelist == MAP_FAILED) {
            printf("--> malloc map failed\n");
            freelist = NULL;
            return NULL;
        } else {
            printf("--> malloc create map\n");
            freelist->size = 2048 - sizeof(m_header);
            freelist->prev = NULL;
            freelist->next = NULL;
            freelist->in_use = 0;
        }
    }
    print_freelist();

    m_header* curr = freelist;

    while(curr != NULL) {
        printf("--> malloc Checking %p: size:%lu prev:%p next:%p use:%d\n", curr, curr->size, curr->prev, curr->next, curr->in_use);
        if (curr->size >= size && curr->in_use == 0) { // pick first one that is big enough
            printf("--> malloc Found %p: size:%lu prev:%p next:%p use:%d\n", curr, curr->size, curr->prev, curr->next, curr->in_use);
            curr->in_use = 1;
            // Break into smaller chunks only if the leftover space is enough to hold a new header and at least 16 bytes of data
            printf("--> malloc curr->size=%lu size=%lu sizeof(m_header)=%lu neededsize=%lu break=%d\n", curr->size, size, sizeof(m_header), size + sizeof(m_header) + 16, curr->size >= size + sizeof(m_header) + 16);
            if (curr->size >= size + sizeof(m_header) + 16) {
                // Break into smaller chunks

                int newSize = curr->size - size - sizeof(m_header);

                curr-> size = size;
                
                printf("--> malloc Breaking into smaller chunks.\n");
                m_header* newHeader = (m_header*)((char*)curr + size + sizeof(m_header));
                printf("--> malloc newHeader=%p\n", newHeader);
                
                newHeader->size = newSize;

                newHeader->in_use = 0;
                newHeader->prev = curr;
                newHeader->next = curr->next;
                
                curr->next = newHeader;
            }

            print_freelist();
            printf("--> malloc success %p\n", curr);
            return curr + sizeof(m_header); // return pointer to memory after the header
        }
        curr = curr->next;
    }
    printf("--> malloc failed.  Returning NULL\n");
    return NULL;
}

void new_free(void * ptr) {

    printf("\n--> free start.\n");

    m_header* headerPtr = (m_header*)ptr - sizeof(m_header);
    printf("--> Freeing %p: size:%lu prev:%p next:%p use:%d\n", headerPtr, headerPtr->size, headerPtr->prev, headerPtr->next, headerPtr->in_use);
    headerPtr->in_use = 0;

    printf("--> free end.\n");
}
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "malloc.h"

#define ALIGNMENT(size) (((size) + 0xf) & ~(size_t)0xf)

//also when a space is freed it should check if it can fit inside the freed space

void *malloc(size_t size){
    struct malloc_chunk *chunk = 0;
    if (size == 0){
        return NULL;
    }
    //this make is 16 byte aligned
    size  = ALIGNMENT(size+META_SIZE);
    if (!global_base){
        chunk = request_space(size);
        if (!chunk){
            return NULL;
        }
        global_base = chunk;
        global_tail = global_base;
    }else{
        chunk = from_bin(size);
        if (!chunk){
            chunk = request_space(size);
            if (!chunk){
                return NULL;
            }
            chunk->previous = global_tail;
            chunk->previous->next = chunk;
            global_tail = chunk;
        }
        
    }
    return (chunk+1);
}

void free(void *ptr){
    if (!ptr){
        return;
    }

    struct malloc_chunk* control = (struct malloc_chunk*)((char*)ptr - META_SIZE); //ptr points to the given malloc_chunk
    if ((control->size & 0x1) == 0){
        return;
    }
    control->size = control->size & ~0x0f; //the block size will have 0 in the last nibble of the size to indicate that it is not in use
    
    if ((control->next) && (control->next->size & 0x1) == 0){
        remove_from_bin(control->next);
        //coalescing
        control->size += control->next->size;
        control->next = control->next->next;
        if (control->next){
            control->next->previous = control;
        }else{
            global_tail = control;
        }
    }
    if ((control->previous) && (control->previous->size & 0x1) == 0){
        remove_from_bin(control->previous);
        //coalescing
        control->previous->size += control->size;
        control->previous->next = control->next;
        if (control->next){
            control->next->previous = control->previous;
        }else {
            global_tail = control->previous;
        }
        control = control->previous;
    }  
    to_bin(control);  
}

void to_bin(struct malloc_chunk* control){
    if(!control || control->size>=1024){
        return;
    }
    size_t i = (control->size -32)/16;
    control->fd = smallbin[i];
    smallbin[i] = control;
}

struct malloc_chunk *from_bin(size_t size){
    size_t i = 0;
    struct malloc_chunk* tmp;
    if (size>=1024){
        return NULL;
    }
    i = (size-32)/16;
    if (smallbin[i]==0){
        for (;i < NUM_BINS; i++){
            if (smallbin[i]!=0){
                break;
            }
        }
        if (i==NUM_BINS){
            return NULL;
        }
    }
    
    tmp = smallbin[i];
    smallbin[i] = tmp->fd;
    tmp->size |= 0x1;
    return tmp; 
}

void remove_from_bin(struct malloc_chunk *target){
    if (!target || target->size >= 1024){
        return;
    }
    size_t i = (target->size - 32) / 16;
    struct malloc_chunk *curr = smallbin[i];
    struct malloc_chunk *prev = NULL;

    while (curr) {
        if (curr == target) {
            if (prev) {
                prev->fd = curr->fd;
            } else {
                smallbin[i] = curr->fd;
            }
            curr->fd = NULL;
            return;
        }
        prev = curr;
        curr = curr->fd;
    }
}

struct malloc_chunk *request_space(size_t size){
    struct malloc_chunk *block;
    block = sbrk(0);
    void *request = sbrk(size);
    assert((void*)block == request);
    if (request==(void*)-1){
        return NULL;
    }
    block->next = NULL;
    block->previous = NULL;
    size = size | 0x01;
    block->size = size; 
    return block;
}

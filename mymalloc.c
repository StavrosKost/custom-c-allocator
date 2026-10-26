#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "malloc.h"

#define ALIGNMENT(size) (((size) + 0xf) & ~(size_t)0xf)

//also when a space is freed it should check if it can fit inside the freed space

void *malloc(size_t size){
    struct malloc_chunk *chunk;
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
    }else{
        struct malloc_chunk* tmp = chunk;
        chunk = from_bin(size);
        if (!chunk){
            chunk = request_space(size);
            if (!chunk){
                return NULL;
            }
        }
        chunk->previous = tmp;
        chunk->previous->next = chunk;
    }
    return (chunk+1);
}

void free(void *ptr){
    if (!ptr){
        return;
    }

    struct malloc_chunk* control = (struct malloc_chunk*)((char*)ptr - META_SIZE); //ptr points to the given malloc_chunk
    control->size = control->size & ~0x0f; //the block size will have 0 in the last nibble of the size to indicate that it is not in use
    
    if (control->next){
        if ((control->next->size & 0x1) == 0){
            //coalescing
            control->size += control->next->size;
            control->next = control->next->next;
            if (control->next){
                control->next->previous = control;
            }
        }
    }
    if (control->previous){
        if ((control->previous->size & 0x1) == 0){
            //coalescing
            control->previous->size += control->size;
            control->previous->next = control->next;
            if (control->next){
                control->next->previous = control->previous;
            }
            control = control->previous;
        }
    }  
    to_bin(control, control->size);  
}

void to_bin(struct malloc_chunk* control,size_t size){
    if(!control || size>=1024){
        return;
    }
    size_t i = (size-32)/16;
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
    smallbin[i] = tmp->next;
    tmp->size |= 0x1;
    return tmp; 
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

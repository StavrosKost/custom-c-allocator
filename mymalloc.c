#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

struct malloc_chunk{
    size_t size_previous_chunk;
    size_t size;
    struct malloc_chunk* next;
    struct malloc_chunk* previous;
    char data[];
};

#define META_SIZE sizeof(struct malloc_chunk)

void *global_base = NULL;
void *global_tail = NULL;

struct malloc_chunk* smallbin[62];
struct malloc_chunk *request_space(size_t size);
struct malloc_chunk *to_bin(struct malloc_chunk* control,size_t size);
struct malloc_chunk *from_bin(size_t size);

void free(void *ptr);

void *malloc(size_t size){
    struct malloc_chunk *chunk;
    if (size<=0){
        return NULL;
    }
    //this make is 16 byte aligned
    while((size+META_SIZE)%16!=0){
        size++;
    }
    size = size + META_SIZE;
    if (!global_base){
        chunk = request_space(size);
        if (!chunk){
            return NULL;
        }
        global_base = chunk;
        global_tail = global_base;
    }else{
        struct malloc_chunk *last = global_tail;
        chunk = from_bin(size);
        if (!chunk){
            chunk = request_space(size);
            if (!chunk){
                return NULL;
            }
        }
    }
    
    return (chunk+1);
}

void *calloc(size_t nelem, size_t elsize){
    size_t size = nelem * elsize;
    void *ptr = malloc(size);
    if (ptr){
        memset(ptr, 0 ,size);
    }
    return ptr;
}

void free(void *ptr){
    if (!ptr){
        return;
    }

    struct malloc_chunk* control = (struct malloc_chunk*)((char*)ptr - META_SIZE); //ptr points to the given malloc_chunk
    control->size = control->size & ~0x0f; //the block size will have 0 in the last nibble of the size to indicate that it is not in use
    to_bin(control, control->size);
}

struct malloc_chunk *to_bin(struct malloc_chunk* control,size_t size){
    if(!control || size>=1024){
        return NULL;
    }
    size_t i = 0;
    i = (size-32)/16;
    control->next = smallbin[i];
    smallbin[i] = control;
    return smallbin[i]; 
}

struct malloc_chunk *from_bin(size_t size){
    size_t i = 0;
    struct malloc_chunk* tmp;
    if (size>=1024){
        return NULL;
    }
    i = (size-32)/16;
    if (smallbin[i]==0){
        return NULL;
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
    size = size | 0x01;
    block->size = size; 
    global_tail = block;
    return block;
}

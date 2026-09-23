struct malloc_chunk{
    size_t size_previous_chunk;
    size_t size;
    struct malloc_chunk* next;
    struct malloc_chunk* previous;
    char data[];
};

#define META_SIZE sizeof(struct malloc_chunk)

void *global_base = NULL;

struct malloc_chunk* smallbin[62];
struct malloc_chunk *request_space(size_t size);
struct malloc_chunk *to_bin(struct malloc_chunk* control,size_t size);
struct malloc_chunk *from_bin(size_t size);

void free(void *ptr);

void *malloc(size_t size);

struct malloc_chunk *to_bin(struct malloc_chunk* control,size_t size);
struct malloc_chunk *from_bin(size_t size);

struct malloc_chunk *request_space(size_t size);

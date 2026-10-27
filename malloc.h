struct malloc_chunk{
    size_t size_previous_chunk;
    size_t size;
    struct malloc_chunk* next;
    struct malloc_chunk* previous;
    struct malloc_chunk* fd;
    struct malloc_chunk* bk;
    char data[];
};

#define META_SIZE sizeof(struct malloc_chunk)
#define NUM_BINS 62

void *global_base = NULL;
void *global_tail = NULL;

struct malloc_chunk* smallbin[NUM_BINS];
struct malloc_chunk *request_space(size_t size);
struct malloc_chunk *from_bin(size_t size);

void to_bin(struct malloc_chunk* control);
void remove_from_bin(struct malloc_chunk *target);
void free(void *ptr);
void *malloc(size_t size);

# Custom C Allocator
Developed a custom C allocator which mimics the way glibc allocates data using malloc and also frees data using free. This project manages heap that is allocated using sbrk system call and is build with small bins to reduce fragmentation that is created via linear allocation uisng double linked lists.

## Allocation
- The user requests some size to be allocated, that space is initially aligned in multiples of 16
- If the total size is less that 1024 bytes, the allocator calculates the index that correspondes to the most appropriate placement inside the small bin.
- Alternatively, the allocator requests space from the kernel using sbrk to physically extend the available space.

## Free
- When the free is called the meta data are initially subtracted in order to be on chunk header
- It clears the in-use bit of the size, calculates the bin index and pushes the chunk to the appropriate smallbin

## Usage
```
gcc -shared -fPIC -o libmymalloc.so mymalloc.c
LD_PRELOAD=./libmymalloc.so ./program

or

gcc test.c mymalloc.c -o my_program
```

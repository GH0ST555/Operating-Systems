//contains the metadata for the heap
//also contains information of the next and previous nodes of our linked list structure
typedef struct metadata{
    int heap_size;
    struct metadata *next;
    struct metadata *prev;
} heap;

//function prototypes
void * _malloc(int size);
void _free(void *ptr);
heap *splt(heap *ptr, int size);
void rem_blk(heap *ptr);
void blk_add(heap *ptr);


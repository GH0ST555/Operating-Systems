
#include "kernel/types.h"
#include "user/user.h"
#include "memory_management.h"


//declare fixed values which are used in malloc and free .. incremented and decremented with respect to size of heap
#define mem_size(ptr) ((void *)(ptr + sizeof(heap)))
#define hdr_size(ptr) ((void *)(ptr - sizeof(heap)))

//declare the header
heap *header =0;

//function that removes the block from the free list
//adjusts the header of linked list
void rem_blk(heap *ptr){
    if (!ptr->prev) {
		if (ptr->next) {
			header = ptr->next;
		} else {
			header = 0;
		}
	} 
    else {
		ptr->prev->next = ptr->next;
	}
	if (ptr->next) {
		ptr->next->prev = ptr->prev;
	}
}   
    
//adds block to the free list and list is sorted by address
//useful to scan continuous blocks
void blk_add(heap *ptr){
    //initialize link list pointers
    ptr->next =0;
    ptr->prev = 0;
    //conditions for failed header or if pointer is greater 
    if(!header || ptr < header){
        if(header){
            header->prev = ptr;    
        }
        ptr->next = header;
        header = ptr;
        
    }

    else{
        heap * curnt = header;
        while(curnt->next && curnt->next < ptr){
            
            curnt =curnt->next;
        }
        ptr->next = curnt->next;
        curnt->next = ptr;
    }

}

//splits block into size and the remaing bit 
heap *splt(heap *ptr, int size){
    void *m_blk = mem_size(ptr); 
    heap  *nptr = (heap*)(m_blk+size);
    nptr->heap_size = ptr->heap_size - (size + sizeof(heap));
    ptr->heap_size = size;
    return nptr;
}


//returns a pointer that points to the heap ,with the requested size and performs a system call to expand heap size if necessary
// must check free list using 
void * _malloc(int size){
    if (size == 0){
        return 0;
    }
    //initialize variables
    void* blk_m;
    heap *ptr, *nptr;
    //sets the initial sbrk size.. i have set this to a large value to reduce the number of sbrk() calls
    int set_size = 5000 + size;
    ptr = header;
    //when malloc is run after its first call
    while(ptr){
        //checks if the block is large enough
        if (ptr -> heap_size >= size + sizeof(heap)){
            //remove it from free list
            blk_m= mem_size(ptr);
            rem_blk(ptr);
            //if it is the exact size .. dont split
            if(ptr->heap_size == size){
                return blk_m;
            }
            //split the block into required size and return pointer
            nptr = splt(ptr,size);
            blk_add(nptr);
            return blk_m;
        }
        else{
            ptr = ptr -> next;
        }
    }
    ptr = (heap*)sbrk(set_size);
    if(ptr == (void*) -1){//sbrk failed
        return 0;
    }
    //when malloc is called for the first time
    //initialize the heap
    ptr->next = 0;
    ptr->prev = 0;
    ptr->heap_size = set_size - sizeof(heap);
    if(set_size > size + sizeof(heap)){
        nptr = splt(ptr,size);
        blk_add(nptr);
    }
    return mem_size(ptr);

}

//adds the pointer to free list 
//also scans and merges the free blocks
void _free(void * ptr){
    //checks for invalid pointer if so do not free it
    if(!ptr || ptr > (void*)sbrk(0)){
        return;
    }
    //if pointer is valid
    //intialize variables for scanning and merging of blocks
    blk_add(hdr_size(ptr));
    heap* curnt = header;    
    unsigned long	hdr_currnt, hdr_next;
    unsigned long	pb = (unsigned long)sbrk(0);
	
    if (pb == 0){
        return;
    }

    //this code section does the scanning and merging of blocks
    //checks free list to find adjacent free blocks and merge them 
    //scans through linked list

    while (curnt->next){
        hdr_currnt = (unsigned long)curnt;
        hdr_next = (unsigned long)curnt->next;
        //if current block size and heap size is equal to the  next block 
        if (curnt->heap_size+ hdr_currnt + sizeof(heap) + 1 == hdr_next){
            //set the new merged size

            curnt->heap_size = curnt->heap_size + curnt->next->heap_size + sizeof(heap) + 1;

            curnt->next = curnt->next->next;
            if(curnt->next){
                curnt->next->prev = curnt;
            }
            else{
                break;
            }
        }
        curnt = curnt->next;
    }
    //checks if the last block is big enough to be freed
    hdr_currnt = (unsigned long)curnt;
    if(hdr_currnt + curnt->heap_size + sizeof(heap) + 1 == pb){
        rem_blk(curnt);
    }
}

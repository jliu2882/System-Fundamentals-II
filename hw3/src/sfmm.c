/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "sfmm.h"
#include "debug.h"

//TODO
//Implement sf_malloc, sf_free, sf_realloc, and then write 5 Criterion tests

//OOGA BOOGA TODO
//Check for places whjere I asuume iunitial value of 0
//create  afunction to tget footer
//Helpful functions in stutil can be found in sfmm.h

//Static variables
static size_t minBlockSize = 8*sizeof(sf_header); //Record minimum block size here
static int initialized = 0; //Keep a record of if we initialized memory yet
static sf_block *prologue; //Keep track of the prologue block; could optimize by pointing to next block
static sf_block *epilogue; //Keep track of the epilogue block; will be updated as we call sf_mem_grow
static size_t sizeClass[NUM_FREE_LISTS]; //Keep track of the size classes since we can't edit sentinel's data


//Helper functions that replaces repeated functionality or to make the code easier to read
static size_t roundUp(size_t size, size_t multiple); //Round the size up to the nearest block size
static void *getFreeBlock(size_t size); //Find a free block, allocate it, and return a pointer to the payload
static int initializeMemory(); //Initialize the free list heads, prologue and epilogue
static sf_block *addPointer(sf_block *pp, int size); //Cast the pointer to void* to add by byte and cast back to sf_block*
static void setHeader(sf_block *pp, size_t header); //Set the header of a block and if it's a free block, set the footer as well
static sf_block *getNextBlock(sf_block *pp); //Get the next block, which can allow access to prev_footer
static int getSizeOfBlock(sf_block *pp); //Get the size of a given sf_block
static void placeBlock(sf_block *pp); //Insert into proper free list, assumes block is coalesced and everything is set
static void setEpilogue(); //Set the epilogue for the current sf_mem_end()
static int findFreeListIndex(size_t size); //Find the index of the smallest free list that holds size class size
static int requestMemory(size_t size); //Request more memory coalescing if appropiate(Wrapper for sf_mem_grow)
static void freeBlock(sf_block *pp); //Free the block by erasing any corresponding allocated bits and coalesce if possible
static sf_block *coalesceBlock(sf_block *pp); //Coalesce the block with adjacent free blocks, assumes initial block is free

    //next up
static void splitBlock(void *pp, size_t newSize); //Split the block if possible while keeping pp a valid block
    //not done or used yet
static void verifyPointer(void *pp); //Abort the program if the pointer is invalid
static sf_block *findFreeList(void *pp); //?return is probably wrong


//Main functions
void *sf_malloc(size_t size) { //Note that free blocks need at least 32 bytes which rounds to a block of 64 bytes
    if(size == 0) return NULL; //If size is 0, return NULL without setting sf_errno
    size_t minSize = 8 + size; //The size is 8 bytes for the header plus the requested payload size
    minSize = roundUp(minSize, minBlockSize); //Round to the next multiple of 64 bytes (AKA the block size)
    return getFreeBlock(minSize); //Get a pointer to the payload, calling sf_mem_grow/split and setting sf_errno if necessary
} //return size==0?NULL:findFreeBlock(roundUp(8+size,64)); //Technically works but one liner < readability

void sf_free(void *pp) {
    verifyPointer(pp); //Verify the pointer and stop the program if the pointer is invalid
    //freeBlock(pp); //Free the block and coalesce if appropiate
    //freeList = findFreeList(pp); //Find a free list for our pointer
    //placeBlock(pp,freeList); //Place the block into the free list
    return; //not needed so delete later :))
}

void *sf_realloc(void *pp, size_t rsize) {
    //maybe use this as case in splitBlock to 0
    if(rsize==0){ //If size is 0, we want to free the block and allocate nothing
        sf_free(pp); //sf_free will check if the pointer is valid
        return 0; //Return 0 to let the program know we didn't find any errors
    }
    verifyPointer(pp); //Verify the pointer and stop the program if the pointer is invalid
    //if(rsize>size of pp){ //!Calculate size of pp first
        //if((newBlock=sf_malloc(rsize))==NULL) //Assign a newBlock of the proper size
            //return NULL; //sf_errno will be set if sf_malloc deems it appropiate
        //!;Call memcpy to just copy the whole payload
        //sf_free(pp); //Free the old block
        //return newBlock; //Return the newly allocated block
    //} else{ //Technically we should have a case of equal size but it works
        //splitBlock(pp,rsize); //Split the block if possible otherwise do nothing
        //return pp; //The payload is still the same just cut off if applicable
    //}
    return NULL; //There is no scenario where this should be called
}


//Implementation of helper functions
static size_t roundUp(size_t size, size_t multiple){ //Round the size up to the nearest block size
    size_t remainder = size % multiple; //Determine if we are already a multiple(the distance to the previous multiple)
    if (remainder == 0) return size; //If we are, just return the size and there is no need to round up
    return size - remainder + multiple; //Otherwise, move down to the next multiple and then go to the next multiple
} //Simple self-explanatory function really

static void *getFreeBlock(size_t size){ //Find a free block, allocate it, and return a pointer to the payload
    if(initialized == 0) //If the memory is uninitialized, initialize it first then continue
        if(initializeMemory()) return NULL; //initializeMemory will set sf_errno if appropriate
    int i = findFreeListIndex(size); //Start at the first size class that is possibly valid
    while(sf_free_list_heads[i].body.links.next == &sf_free_list_heads[i]){ //...prev is the same if it is properly circular
        if(++i==NUM_FREE_LISTS){ //Move to the next size class and check if we WERE on the last list at index NUM_FREE_LIST-1
            if(requestMemory(size)) return NULL; //requestMemory will return nonzero on failures and set sf_errno for us
            i--; //Subtract i since we went past the max index
            break; //Break the for loop so we don't increment i again
        } //We will always increment i, and we use pre-increment so we can compare with NUM_FREE_LIST vs NUM_FREE_LIST-1
    } //After the while loop, i is the index of a freelist with a big enough block for size


//    sf_block *freeBlock = sf_free_list_heads[i].body.links.next; //Get the pointer to the block from the free list
//sf_show_block(freeBlock);
    //splitBlock(freeBlock, size); //Split the block we found to the proper size
    //return sf_free_list_heads[i].body; //or smth idk return the payload
sf_show_heap(); //show heap as is


    return NULL; //temp so i dont get yelled at
} //Note that this function returns either NULL or a pointer to the payload

static int initializeMemory(){ //Initialize the free list heads, prologue and epilogue
    initialized = 1; //We initialized the memory, so we don't want to do it again
    if(sf_mem_grow()==NULL) return 1; //Request a page of memory and return an error and set sf_errno is applicable
    sf_block *start = ((sf_block *)sf_mem_start()); //Get the starting address of the heap and cast it to sf_block*
    size_t padding = minBlockSize - sizeof(sf_header) - sizeof(sf_footer); //Start on aligned address giving room for header/footer
    prologue = addPointer(start, padding); //Add the padding to the block
    setHeader(prologue,minBlockSize+THIS_BLOCK_ALLOCATED); //Set the header to the minimum size of a block in bits
    int fib1=1, fib2=1, temp; //We want to have fibonacci numbers as our size classes
    for(int i = 0; i < NUM_FREE_LISTS; i++){ //Make a sentinel for each size class as well as define the size classes
        sizeClass[i]=fib2; //The max blocks in class i is fib2; Note that the previous size class necessarily limits the range
        sf_block *current = &sf_free_list_heads[i]; //Get the sentinel node for the current size class
        current->body.links.next = current; //Initialize it to point to itself
        current->body.links.prev = current; //This is a circular doubly linked list
        temp=fib1; //Classic Fibonacci sequence for the size classes, storing the value and using it to get the next
        fib1=fib2; //The reason we put this here instead of in the for loop as multiple increments is due to readability
        fib2=temp+fib1; //Get the next size class; Note that the last size class doesn't matter since it has everything else
    } //Initialized the free lists; Note that the rest of the node is NOT used to contain data(I think it's more efficient)
    size_t blockSize = PAGE_SZ-2*minBlockSize; //We use two blocks from page(Mostly on prologue+alignment but 8 bytes on epilogue)
    blockSize+=PREV_BLOCK_ALLOCATED; //The previous block is allocated as it is our prologue and the current block is free
    sf_block *block = addPointer(prologue,minBlockSize); //The first block is a block size after our prologue
    setHeader(block, blockSize); //Set the header/footer of the block and since this is free, the footer as well
    placeBlock(block); //Place the block in its proper free list
    setEpilogue(); //Set the epilogue
    return 0; //Successfully initialized memory without an error
} //Do all the initialization here and then some just in case :)

static sf_block *addPointer(sf_block *pp, int size){ //Cast the pointer to void* to add by byte and cast back to sf_block*
    void *ppClone = (void *)pp; //Cast pp to a void* for easier math
    ppClone+=size; //Add the size to the end
    return ((sf_block *)ppClone); //Cast back to sf_block* for easier use
} //We use int since we don't expect large additions and this allows for subtraction

static void setHeader(sf_block *pp, size_t header){ //Set the header of a block and if it's a free block, set the footer as well
    //read the free bit and if its free then set footer as well
    pp->header=header; //Set the header of the block
    if(!(pp->header&THIS_BLOCK_ALLOCATED)){ //If the block is free, we want to set a footer as well
        pp=getNextBlock(pp); //Get the next block, which can allow access to prev_footer
        pp->prev_footer=header; //Set the prev_footer of the next block to the header
    } //If the block was allocated, we won't mess with the payload
} //Created a function to deal with headers/footers to avoid confusions

static sf_block *getNextBlock(sf_block *pp){ //Get the next block, which can allow access to prev_footer
    return addPointer(pp,getSizeOfBlock(pp)); //Add the size of the block to reach the footer
} //This is here to increase readability of the code

static int getSizeOfBlock(sf_block *pp){ //Get the size of a given sf_block
    return pp->header & ~THIS_BLOCK_ALLOCATED & ~PREV_BLOCK_ALLOCATED; //Mask out the 2 bits set aside for other uses
} //Note that the other 4 unused bits will be 0 so there's no need to mask them out

static void placeBlock(sf_block *pp){ //Insert into proper free list, assumes block is coalesced and everything is set
    int size = getSizeOfBlock(pp); //Get the size of the block we are trying to insert
    sf_block *freeList = &sf_free_list_heads[findFreeListIndex(size)]; //Find the proper sentinel node
    freeList->body.links.next->body.links.prev=pp; //The current first block should point back to the new
    pp->body.links.next=freeList->body.links.next; //Our block should point to the old first block
    freeList->body.links.next=pp; //Our block should be the new first block
    pp->body.links.prev=freeList; //We also want it to point back to the sentinel node
} //Insert a block of memory into the proper free list; Note that we shouldn't have to change the header/footer

static void setEpilogue(){ //Set the epilogue for the current sf_mem_end()
    void *end = sf_mem_end(); //Get the ending address of the heap using void* so we can add by bytes to the address
    epilogue=addPointer(end,-(int)(sizeof(sf_header)+sizeof(sf_footer))); //Move back the bytes for header/footer
    setHeader(epilogue,THIS_BLOCK_ALLOCATED); //Set the epilogue to just the allocated bit; previous is the new free block
} //Used for modularity but idk maybe it's too modular now

static int findFreeListIndex(size_t size){ //Find the index of the smallest free list that holds size class size
    for(int i = 0; i < NUM_FREE_LISTS; i++) //Loop through the free lists to find the first list it belongs in
        if(size<=sizeClass[i]) return i; //The size can fit within the maximum of size class i
    return NUM_FREE_LISTS-1; //If it wasn't less than a size class it must be in the last free list which emcompasses infinity
} //Note that sizeClass[NUM_FREE_LISTS-1] isn't accurate for the max size, but the final return value anticipates this

static int requestMemory(size_t size){ //Request more memory coalescing if appropiate(Wrapper for sf_mem_grow)
    int pagesNeeded = (size/PAGE_SZ)+(size%PAGE_SZ!=0); //Integer division rounds down so add a page if it wasn't a perfect fit
    for(int i = 0; i < pagesNeeded; i++){ //Request pages of memory until we have enough memory for a block of size
        if(sf_mem_grow()==NULL) return 1; //Add a page of memory; Note that sf_errno will be set on errors
        int newMemory = PAGE_SZ - sizeof(sf_header); //We need to give space for the new epilogue, but otherwise we're good
        newMemory+=(PREV_BLOCK_ALLOCATED&epilogue->header); //We want to keep prev_alloc and we'll erase alloc later
        sf_block *newBlock = epilogue; //Save the address of the epilogue so we can move epilogue
        setHeader(newBlock,newMemory); //Set the header of the new block
        setEpilogue(); //Set the epilogue(vs epilogue=getNextBlock(epilogue); setHeader(epilogue,THIS_BLOCK_ALLOCATED);)
        freeBlock(newBlock); //Erase the bit in alloc and the prev_alloc of the next block and coalesce with adjacent blocks
    } //Requested the pages of memory successfully; Note that for ^^ the next block is allocated already but we will still check
    return 0; //Successfully request more memory without any errors
} //Created a wrapper around sf_mem_grow which asks for more memory and does the needed changes

static void freeBlock(sf_block *pp){ //Free the block by erasing any corresponding allocated bits and coalesce if possible
    pp->header&=~THIS_BLOCK_ALLOCATED; //Unallocate the alloc bit of the current block
    getNextBlock(pp)->prev_footer&=~PREV_BLOCK_ALLOCATED; //Unallocate the prev_alloc bit of the next block
    coalesceBlock(pp); //Coalesce the block if possible but we don't need to recognize it's return value
} //This function frees the block and places it in the free list

static sf_block *coalesceBlock(sf_block *pp){ //Coalesce the block with adjacent free blocks, assumes initial block is free
    while(!(pp->header&PREV_BLOCK_ALLOCATED //Note that we didn't use pp->prev_footer because it might be allocated and thus garbage
        && getNextBlock(pp)->header&THIS_BLOCK_ALLOCATED)){ //Check if any of the adjacent blocks are free

        //combo wombo it with neighbors
        //return the new pointer
            //The exception is if they are the beginning or end of the heap
        //update pointer to pp

    } //There are no more adjacent free blocks
    placeBlock(pp); //Place the block in it's proper free list
    return pp; //The block is freed and shouldn't be touched but in case of emergency idk
} //Created the return value before I got this far and I'm too lazy to change so return it does :)


static void splitBlock(void *pp, size_t newSize){ //Split the block if possible while keeping pp a valid block
    //If a splinter(block of less than 64) will happen, we don't split and return pp
        //To be explicit, if (size of pp)-newSize<64, splitting leads to a splinter(catches newSize>pp)
    //?Create a header/etc for the upper half
    //update the header/etc..
    //list=findFreeList(upperHalf)
    //placeBlock(upperHalf,list)
}




static void verifyPointer(void* pp){ //Abort the program if the pointer is invalid
    if(pp==NULL) //If the pointer is NULL, it is invalid
        abort(); //The pointer is invalid, so we should abort the program
    if((uintptr_t)pp%minBlockSize==0)//If the pointer is not 64-byte aligned, it is invalid
    //Check actually
        abort(); //The pointer is invalid, so we should abort the program
    //If the pointer is NULL
    //If the pointer is not 64 byte aligned
    //If the header is before the start of heap, or the footer is after the end of heap
    //If the allocated bit is 0
    //If the prev_alloc field is 0, but the previous block's alloc field is not 0
    return; //We successfully validated the pointer; This implementation was nicer on the eyes
}

static sf_block *findFreeList(void *pp){ //?return is probably wrong
    //Calculate size from the header of pp
    //return sf_free_list_heads[findFreeListHelper(size)]
    return NULL;
}
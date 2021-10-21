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
static sf_block *addPointer(sf_block *pp, size_t size); //Cast the pointer to char* to add by byte and cast back to sf_block*
static void setHeader(sf_block *pp, size_t header); //Set the header of a block and if it's a free block, set the footer as well
static sf_block *getNextBlock(sf_block *pp); //Get the next block, which can allow access to prev_footer
static size_t getSizeOfBlock(sf_block *pp); //Get the size of a given sf_block
static void placeBlock(sf_block *pp); //Insert into proper free list, assumes block is coalesced and everything is set
static void setEpilogue(size_t prev_bit); //Set the epilogue for the current sf_mem_end() and set the prev bit
static sf_block *subtractPointer(sf_block *pp, size_t size); //Cast the pointer to char* to subtract by byte and cast back to sf_block*
static int findFreeListIndex(size_t size); //Find the index of the smallest free list that holds size class size
static int requestMemory(size_t size); //Request more memory coalescing if appropiate(Wrapper for sf_mem_grow)
static void freeBlock(sf_block *pp); //Free the block by erasing any corresponding allocated bits and coalesce if possible
static sf_block *coalesceBlock(sf_block *pp); //Coalesce the block with adjacent free blocks, assumes initial block is free
static sf_block *getPreviousBlock(sf_block *pp); //Get the previous block, assumes that the block is free
static void removeFromFreeList(sf_block *pp); //Remove a block from a free list
static sf_block *findBlockInList(sf_block *sentinel, size_t size); //Find a block of the right size; NULL if none exist
static void splitBlock(sf_block *pp, size_t newSize); //Split the block if possible while keeping pp a valid block
static void verifyPointer(void *pp); //Abort the program if the pointer is invalid
static int outsideHeap(sf_block *pp); //Determine if the header is before the start of heap, or the footer is after the end of heap


//Main functions
void *sf_malloc(size_t size) { //Note that free blocks need at least 32 bytes which rounds to a block of 64 bytes
    if(size == 0) return NULL; //If size is 0, return NULL without setting sf_errno
    size_t minSize = 8 + size; //The size is 8 bytes for the header plus the requested payload size
    minSize = roundUp(minSize, minBlockSize); //Round to the next multiple of 64 bytes (AKA the block size)
    return getFreeBlock(minSize); //Get a pointer to the payload, calling sf_mem_grow/split and setting sf_errno if necessary
} //return size==0?NULL:findFreeBlock(roundUp(8+size,64)); //Technically works but one liner < readability

void sf_free(void *pp) {
    verifyPointer(pp); //Verify the pointer and stop the program if the pointer is invalid
    pp=(char*)pp-16; //We want to go from the payload to the start of sf_block; cast to char* for portability
    freeBlock(pp); //Free the block and coalesce if appropiate
} //If we made verifyPointer return a pointer; -> freeBlock(verifyPointer(pp-16));

void *sf_realloc(void *pp, size_t rsize) {
    if(rsize==0){ //If size is 0, we want to free the block and allocate nothing
        sf_free(pp); //sf_free will check if the pointer is valid so no need to verify first
        return NULL; //If the pointer is valid but the size parameter is 0, free the block and return NULL.
    } //cool cool cool cool
    verifyPointer(pp); //Verify the pointer and stop the program if the pointer is invalid
    pp=(char*)pp-16; //We want to go from the payload to the start of sf_block; cast to char* for portability
    rsize+=sizeof(sf_header); //The new block needs a header
    if(rsize>getSizeOfBlock(pp)){ //If we are reallocating to a larger block, we want to get a new space
        sf_block *newBlock = sf_malloc(rsize); //Assign a new block with size rsize
        if(newBlock==NULL) return NULL; //sf_errno will be set if sf_malloc deems it appropiate
        size_t payloadSize = getSizeOfBlock(pp)-sizeof(sf_header); //The paylod is the size minus the header(allocated=no footer)
        memcpy(newBlock->body.payload, ((sf_block *)pp)->body.payload, payloadSize); //Copy the payload to the new block
        sf_free(((sf_block *)pp)->body.payload); //Free the old block by sending the payload
        return newBlock->body.payload; //Return the newly allocated block
    } else{ //Technically we should have a case of equal size but it works since split to same size doesn't break
        splitBlock(pp,rsize); //The payload is the same but the way we read it is different
        return ((sf_block *)pp)->body.payload; //The payload is still the same just cut off if applicable
    } //Note that if we do split, the split block will have garbage values; Note that we need room for the header as well
    return NULL; //There is no scenario where this should be called, I think
} //create a function sf_free_helper; return sf_free_helper < MEME BUT LOL


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
    sf_block *foundBlock=NULL; //Uninitialized for now, but will store the proper block; use as a flag
    while(!foundBlock){//go until we find the block
    //} && sf_free_list_heads[i].body.links.next == &sf_free_list_heads[i]){ //...prev is the same if it is properly circular
        foundBlock = findBlockInList(&sf_free_list_heads[i], size); //Find a block of proper size in the free list; NULL if none
        if(++i>=NUM_FREE_LISTS){ //Move to the next size clas; for some reason doesn't work as intended but the fix fixed it
            if(requestMemory(size)) return NULL; //requestMemory will set sf_errno for us; it will put a big enough block first
            foundBlock = sf_free_list_heads[i-1].body.links.next; //Retrieve the block in the first position(!foundBlock=false)
            break; //Break the for loop so we don't increment i again
        } //We will always increment i, and we use pre-increment so we can compare with NUM_FREE_LIST vs NUM_FREE_LIST-1
    } //After the while loop, i is the index of a freelist with a big enough block for size
    if(foundBlock==NULL) return NULL; //This shouldn't happen but it's here for
    removeFromFreeList(foundBlock); //Remove the block from the free list it is in and set the prev bits of the next block
    setHeader(foundBlock, foundBlock->header|THIS_BLOCK_ALLOCATED); //Set the block to the allocated
    splitBlock(foundBlock, size); //Split the block we found to the proper size
    return foundBlock->body.payload; //Return a pointer to the payload of the free block
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
    setEpilogue(0); //Set the epilogue and know that the initial prev should be 0
    return 0; //Successfully initialized memory without an error
} //Do all the initialization here and then some just in case :)

static sf_block *addPointer(sf_block *pp, size_t size){ //Cast the pointer to char* to add by byte and cast back to sf_block*
    char *ppClone = (char *)pp; //Cast pp to a char* for easier math
    ppClone+=size; //Add the size to the end
    return ((sf_block *)ppClone); //Cast back to sf_block* for easier use
} //We use int since we don't expect large additions and this allows for subtraction

static void setHeader(sf_block *pp, size_t header){ //Set the header of a block and if it's a free block, set the footer as well
    pp->header=header; //Set the header of the block
    if(!(pp->header&THIS_BLOCK_ALLOCATED)){ //If the new header of the block is free, we want to set a footer as well
        pp=getNextBlock(pp); //Get the next block, which can allow access to prev_footer
        pp->prev_footer=header; //Set the prev_footer of the next block to the header
    } //If the block was allocated, we won't mess with the payload
} //Created a function to deal with headers/footers to avoid confusions

static sf_block *getNextBlock(sf_block *pp){ //Get the next block, which can allow access to prev_footer
    return addPointer(pp,getSizeOfBlock(pp)); //Add the size of the block to reach the footer
} //This is here to increase readability of the code

static size_t getSizeOfBlock(sf_block *pp){ //Get the size of a given sf_block
    return pp->header & ~THIS_BLOCK_ALLOCATED & ~PREV_BLOCK_ALLOCATED; //Mask out the 2 bits set aside for other uses
} //Note that the other 4 unused bits will be 0 so there's no need to mask them out

static void placeBlock(sf_block *pp){ //Insert into proper free list, assumes block is coalesced and everything is set
    size_t size = getSizeOfBlock(pp); //Get the size of the block we are trying to insert
    sf_block *freeList = &sf_free_list_heads[findFreeListIndex(size)]; //Find the proper sentinel node
    freeList->body.links.next->body.links.prev=pp; //The current first block should point back to the new
    pp->body.links.next=freeList->body.links.next; //Our block should point to the old first block
    freeList->body.links.next=pp; //Our block should be the new first block
    pp->body.links.prev=freeList; //We also want it to point back to the sentinel node
} //Insert a block of memory into the proper free list; Note that we shouldn't have to change the header/footer

static void setEpilogue(size_t prev_bit){ //Set the epilogue for the current sf_mem_end() and set the prev bit
    if(prev_bit) prev_bit = PREV_BLOCK_ALLOCATED; //On any input, we want to set the previous bit
    void *end = sf_mem_end(); //Get the ending address of the heap using void* so we can add by bytes to the address
    epilogue=subtractPointer(end,(sizeof(sf_header)+sizeof(sf_footer))); //Move back the bytes for header/footer
    setHeader(epilogue,THIS_BLOCK_ALLOCATED+prev_bit); //Set the epilogue to the allocated bit; previous depends on the input
} //Used for modularity but idk maybe it's too modular now

static sf_block *subtractPointer(sf_block *pp, size_t size){ //Cast the pointer to char* to subtract by byte and cast back to sf_block*
    char *ppClone = (char *)pp; //Cast pp to a char* for easier math
    ppClone-=size; //Add the size to the end
    return ((sf_block *)ppClone); //Cast back to sf_block* for easier use
} //We use int since we don't expect large additions and this allows for subtraction

static int findFreeListIndex(size_t size){ //Find the index of the smallest free list that holds size class size
    size=(int)(size/64); //We want to find the size in blocks
    for(int i = 0; i < NUM_FREE_LISTS; i++) //Loop through the free lists to find the first list it belongs in
        if(size<=sizeClass[i]) return i; //The size can fit within the maximum of size class i
    return NUM_FREE_LISTS-1; //If it wasn't less than a size class it must be in the last free list which emcompasses infinity
} //Note that sizeClass[NUM_FREE_LISTS-1] isn't accurate for the max size, but the final return value anticipates this

static int requestMemory(size_t size){ //Request more memory coalescing if appropiate(Wrapper for sf_mem_grow)
    int index = findFreeListIndex(size); //Start at the first size class that is possibly valid
    while(index<NUM_FREE_LISTS){ //Check every list for a block
        if(findBlockInList(&sf_free_list_heads[index], size)) return 0; //Stop the function if we got a big enough block
        index++; //Move to the next list
    } //For some reason, it isn't checked so I have this and I'm too lazy to refactor now
    int pagesNeeded = (size/PAGE_SZ)+(size%PAGE_SZ!=0); //Integer division rounds down so add a page if it wasn't a perfect fit
    for(int i = 0; i < pagesNeeded; i++){ //Request pages of memory until we have enough memory for a block of size
        if(sf_mem_grow()==NULL) return 1; //Add a page of memory; Note that sf_errno will be set on errors
        size_t newMemory = PAGE_SZ;// - sizeof(sf_header); //We need to give space for the new epilogue, but otherwise we're good
        newMemory|=(getSizeOfBlock(epilogue)|(PREV_BLOCK_ALLOCATED&epilogue->header)); //We want to keep prev_alloc
        newMemory&=(~THIS_BLOCK_ALLOCATED); // Erase alloc bit now
        sf_block *newBlock = epilogue; //Save the address of the epilogue so we can move epilogue
        setHeader(newBlock,newMemory); //Set the header of the new block
        setEpilogue(THIS_BLOCK_ALLOCATED&epilogue->header); //Set the epilogue(vs epilogue=getNextBlock(epilogue); setHeader(epilogue,THIS_BLOCK_ALLOCATED);)
        freeBlock(newBlock); //Erase the bit in alloc and the prev_alloc of the next block and coalesce with adjacent blocks
        int index = findFreeListIndex(size); //Start at the first size class that is possibly valid
        if(findBlockInList(&sf_free_list_heads[index], size)) break; //Break the loop if we coalesced and got a big enough block
    } //Requested the pages of memory successfully; We will create enough pages until we find a valid block size
    return 0; //Successfully request more memory without any errors
} //Created a wrapper around sf_mem_grow which asks for more memory and does the needed changes

static void freeBlock(sf_block *pp){ //Free the block by erasing any corresponding allocated bits and coalesce if possible
    pp->header&=~THIS_BLOCK_ALLOCATED; //Unallocate the alloc bit of the current block
    setHeader(pp,pp->header); //Set the footer of the our block
    setHeader(getNextBlock(pp),(getNextBlock(pp)->header)&(~PREV_BLOCK_ALLOCATED)); //Unallocate the prev_alloc bit of the next block
    placeBlock(pp); //Place the block in it's proper free list
    coalesceBlock(pp); //Coalesce the block if possible but we don't need to recognize it's return value
} //This function frees the block and places it in the free list

static sf_block *coalesceBlock(sf_block *pp){ //Coalesce the block with adjacent free blocks, assumes initial block is free
    while(!(getNextBlock(pp)->header&THIS_BLOCK_ALLOCATED)){ //Check if the next block is free;  note that pp doesn't change here
        sf_block *nextBlock = getNextBlock(pp); //Save the next block since we will update the value in pp
        size_t totalSize = getSizeOfBlock(pp)+getSizeOfBlock(nextBlock); //Add the two sizes together
        setHeader(pp,totalSize+(pp->header&PREV_BLOCK_ALLOCATED)); //Set the header with the previous of the earlier block
        removeFromFreeList(nextBlock); //Remove the block from free list it is in and set the prev bits of the next block
        setHeader(nextBlock,nextBlock->header|THIS_BLOCK_ALLOCATED); //Set the alloc for the block
    } //There are no more free blocks after pp; Note that the latter header can be ignored because it's part of the payload now
    while(!(pp->header&PREV_BLOCK_ALLOCATED)){ //Check if the previous block is free; note that pp goes to the earlier address
        sf_block *prev = getPreviousBlock(pp); //Create a reference for the previous block
        size_t totalSize = getSizeOfBlock(prev)+getSizeOfBlock(pp); //Add the two sizes together
        setHeader(prev,totalSize+(prev->header&PREV_BLOCK_ALLOCATED)); //Set the header with the previous of the earlier block
        removeFromFreeList(pp); //Remove the block from free list it is in and set the prev bits of the next block
        setHeader(pp,pp->header|THIS_BLOCK_ALLOCATED); //Set the alloc for the block
        pp = prev; //Set the new addres of pp to the be the earlier address
    } //There are no more adjacent free blocks; //Note that we didn't use pp->prev_footer because it might be allocated
    removeFromFreeList(pp); //Remove it from the free list to replace it back in; Note that we assumed the block is in here
    placeBlock(pp); //Place it back in the free list; Reinsert it in the proper size class
    return pp; //The block is freed and shouldn't be touched but in case of emergency idk too lazy to change tbh
} //Note that the beginning and end of the heap are allocated(prologue/epilogue)

static sf_block *getPreviousBlock(sf_block *pp){ //Get the previous block, assumes that the block is free
    return subtractPointer(pp,((pp->prev_footer) & ~THIS_BLOCK_ALLOCATED & ~PREV_BLOCK_ALLOCATED)); //Get the previous block
} //This is here to increase readability of the code
static void removeFromFreeList(sf_block *pp){ //Remove a block from a free list

    pp->body.links.prev->body.links.next=pp->body.links.next; //The previous block should jump to the next
    pp->body.links.next->body.links.prev=pp->body.links.prev; //The next block should back to the previous
} //Note that the payload of pp is essentially garbage with the previous pointers

static sf_block *findBlockInList(sf_block *sentinel, size_t size){ //Find a block of the right size; NULL if none exist
    sf_block *next = sentinel->body.links.next; //We start at the first node and go until we reach the sentinel
    while(next!=sentinel){ //Loop until we reach the sentinel
        if(getSizeOfBlock(next)>=size) return next; //We found a suitable block
        next = next->body.links.next; //Go to the next node
    } //We are using pointer for both so we can just compare pointers
    return NULL; //We reached the end and couldn't find a suitable block
} //Considered making this recursive but decided it was too much effort to bug fix

static void splitBlock(sf_block *pp, size_t newSize){ //Split the block if possible while keeping pp a valid block
    size_t oldSize = getSizeOfBlock(pp); //Save the old size for future reference
    if((((int)oldSize)-newSize)<64) return; //We don't want to split if it will cause a splinter; catches newSize>oldSize
    newSize = roundUp(newSize,minBlockSize); //We want to allocate the enough blocks for the newSize
    size_t newBits = (pp->header&THIS_BLOCK_ALLOCATED)?PREV_BLOCK_ALLOCATED:0; //This block's free, but the prev depends on pp
    size_t newBlockHeader = oldSize-newSize; //The new size of the old block is the remaining blocks
    newBlockHeader+=newBits; //Add the bits for this/prev alloc
    sf_block *newBlock = addPointer(pp,newSize); //Keep a variable for the new block; getNextBlock doesn't work without size
    setHeader(newBlock,newBlockHeader); //Set the header of the new block to the remaining size and the proper bits
    setHeader(getNextBlock(newBlock),getNextBlock(newBlock)->header&(~PREV_BLOCK_ALLOCATED)); //The next block is after free
    placeBlock(newBlock); //Place the newly split free block into the proper size class
    coalesceBlock(newBlock); //Coalesce the block if possible
    size_t oldBits = pp->header&(THIS_BLOCK_ALLOCATED+PREV_BLOCK_ALLOCATED); //Save the old alloc/prev_alloc bits
    size_t oldBlockHeader = newSize+oldBits; //Add the old alloc/prev_alloc bits to the new size
    setHeader(pp,oldBlockHeader); //Keep the old bits while updating size
} //We did new first because old would change the values for new or something idk

static void verifyPointer(void *pp){ //Abort the program if the pointer is invalid
    if(pp==NULL) //If the pointer is NULL, it is invalid
        abort(); //The pointer is invalid, so we should abort the program
    if(((size_t)pp)%minBlockSize!=0)//If the pointer to the payload is not 64-byte aligned, it is invalid(size in bytes)
        abort(); //The pointer is invalid, so we should abort the program
    pp=(char*)pp-16; //We want to go from the payload to the start of sf_block; cast to char* for portability
    sf_block *pointer = (sf_block *)pp; //Cast pp to a block and store it in a variable
    if(outsideHeap(pp)) //If the header is before the start of heap, or the footer is after the end of heap
        abort(); //The pointer is invalid, so we should abort the program
    if(!((pointer->header)&THIS_BLOCK_ALLOCATED)) //If the block is not allocated
        abort(); //The pointer is invalid, so we should abort the program
    if((!((pointer->header)&PREV_BLOCK_ALLOCATED))&& //If the current pointer says the last block is free but the prev block disagrees
        (getPreviousBlock(pointer)->header)&THIS_BLOCK_ALLOCATED) //If the last block is allocated, we have no way to check
        abort(); //The pointer is invalid, so we should abort the program
    return; //We successfully validated the pointer; This implementation was nicer on the eyes than nested if statements
} //The pointer is valid

static int outsideHeap(sf_block *pp){ //Determine if the header is before the start of heap, or the footer is after the end of heap
    if(&(pp->header)<&(getNextBlock(prologue)->header)) //If the block's header is before the header of the first block
        return 1; //Note that the first block is allowed, just not anything before it(including the prologue)
    if(&(pp->header)>=(&epilogue->header)) //If the block starts on the epilogue or later it must go on pas it
        return 1; //Note that if it was before this, it must also be aligned so it doesn't extend past epilogue
    return 0; //We have determined it wasn't outside the heap, so it must inside
} //Basically check if block is within heap; return (....)?1:0 but no thanks
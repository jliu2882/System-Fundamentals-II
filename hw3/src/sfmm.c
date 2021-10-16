/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "sfmm.h"


//Implement sf_malloc, sf_free, sf_realloc, and then write 5 Criterion tests
//Iron out a few details with the seudokode
//Helpful functions in stutil can be found in sfmm.h

//Helper functions to increase modularity
//void verifyPointer(void* pp) //Abort the program if the pointer is invalid
    //If the pointer is NULL
    //If the pointer is not 64 byte aligned
    //If the header is before the start of heap, or the footer is after the end of heap
    //If the allocated bit is 0
    //If the prev_alloc field is 0, but the previous block's alloc field is not 0
        //Abort to exit(man abort to learn more)

//void splitBlock(void *pp, size_t newSize) //Split the block if possible; pp is still a valid block
    //If a splinter(block of less than 64) will happen, we don't split and return pp
        //To be explicit, if (size of pp)-newSize<64, splitting leads to a splinter(catches newSize>pp)
    //?Create a header/etc for the upper half
    //update the header/etc..
    //list=findFreeList(upperHalf)
    //placeBlock(upperHalf,list)

//sf_block *findFreeBlock(size_t size) //?return is probably wrong
    //Get the free list from findFreeListHelper(size)
    //If the next block is itself, this size class is not good enough
    //Otherwise, return the next block
    //Increase size and repeat??
        //Probably look for a better way to implement this
    //Find pointer of first free list size class that can hold this block and search until we find first block
        //If none exist, go to the next class
            //If we don't find any blocks, use sf_mem_grow to request more memory
                //We request a page worth, but can call multiple times if needed
                //If sf_mem_grow returns an error, we will set errno to ENOMEM and return NULL
            //After each call to sf_mem_grow, we want to coalesce with free blocks before and after
                //The exception is if they are the beginning or end of the heap

//sf_block *findFreeList(void *pp) //?return is probably wrong
    //Calculate size from the header of pp
    //return findFreeListHelper(size)

//sf_block *findFreeListHelper(size_t size) //?return is probably wrong
    //Find the free list size class it belongs in
    //Returns the head of the free list

//void placeBlock(void *pp, sf_block *freeList) //Insert into proper free list, assumes block is coalesced
//?parameter is probably wrong
    //Read the size of the block
    //Find the appropiate free list
    //Insert it in the front of the list

//coalesceBlock with adjacent blocks
    //

//freeBlock
    //Free the block by setting the alloc bits/prev_alloc of appropiate blocks, etc...
    //The free list blocks must not have allocated bit set and the footer must be identical to the proper header

void *sf_malloc(size_t size) {
    if(size==0){ //If size is 0, return NULL without setting errno
        return NULL;
    }
    //Determine minSize of the block by adding header and padding to reach multiple of 64(align)
        //Should be able to store next and prev pointer when block is free(in payload)
    //pointer=findFreeBlock(minSize)
    //splitBlock(pointer,minSize)
    //return pointer

    return NULL; //temp so code is valid
}

void sf_free(void *pp) {
    //verifyPointer(pp)
    //freeBlock(pp)
    //coalesceBlock(pp)
    //freeList = findFreeList(pp)
    //placeBlock(pp,freeList)
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    if(rsize==0){ //If size is 0, we want to free the block and allocate nothing
        sf_free(pp); //sf_free will check if the pointer is valid
        return 0; //Return 0 to let the progra know we didn't find any errors
    }
    //verifyPointer(pp) //in case size isnt 0
    //rsize>size of pp
        //if((newBlock=sf_malloc(rsize))==NULL)
            //return NULL //errno is set if it needs to, otherwise its fine anyway
        //Call memcpy to just copy the whole payload
        //sf_free(pp)
        //return newBlock
    //rsize=size of pp
        //Should be clear to just return pp
    //rsize<size of pp
        //splitBlock(pp,rsize) //split if possible otherwise do nothing
        //return pointer to payload to smaller pp block(return pp?)
    return NULL;
}
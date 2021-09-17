/*
 * MTF: Encode/decode using "move-to-front" transform and Fibonacci encoding.
 */

#include <stdlib.h>
#include <stdio.h>

#include "mtft.h"
#include "debug.h"

//debugging tools, too lazy to change debug, so I just did printf to show
static void printNode(MTF_NODE *node){
    printf("\nnode=%p,childL=%p,childR=%p,parent=%p,countL=%d,countR=%d,sym=%d/%x"
        ,node, node->left_child, node->right_child, node->parent,
         node->left_count, node->right_count, node->symbol, node->symbol);
}
static int count = 10;
static void printTreeHelper(MTF_NODE *node, int space){
    if(!node)
        return;
    space += count;
    printf("\n");
    printTreeHelper(node->right_child, space);
    for(int i = count; i < space; i++){
        printf(" ");
    }
    printNode(node);
    printTreeHelper(node->left_child, space);
}

static void printTree(MTF_NODE *node){
    printNode(mtf_map);
    printTreeHelper(mtf_map,0);
}
//will be deleted anyway so hm



static int initialized; //Keep track of if everything has been initialized or not
static int depth; //Keep track of the depth of the tree(asked in OH if declaring static variables was ok)
static int powerOfTwo; //I chose this rather than a lg/ceil approach since I'm declaring a variable for depth
static long long int codeNum; //Keep track of the current bits encoded; overkill but we should store codeNum
static int codeLen; //Length of the Fibonacci encoding to ensure we stay under 8 bits(Any bits past this, we ignore)
static int maxRank; //Keep track of the greatest rank allowable for mtf_map_decode

static int strEquals(char *str1, char *str2) { //Compare two strings and returns true if they were equal
    for(; (*str1)!='\0'; str1++,str2++){ //Iterate through str1 until we reach the null/end
        if((*str1)!=*(str2)){ //Compare the current characters in both string
            return 0; //The characters were not equal so the string must not be
        } //The current chracters of each string were equal, so we want to move on
    } //This loop catches all cases ONLY when str1 is shorter than str2
    return (*str2)=='\0'; //If str2 is longer/did not reach the end, it must not be the same string
} //Found some ways to (maybe) make the code more efficient(and harder to read but comments I guess)

static int getPowerOfTwo(int num){ //Gets two to the power of num
    int power = 1; //Assuming the lowest number passed to it will be a 0
    for(int i = 0; i < num; i++){ //We want to multiply for the power
        power*=2; //double the product until we reach the power of 2
    }
    return power; //Returns after successive multiplication
}

static MTF_NODE* getNodePointer(){ //Returns a node from the linked list-esque structure we can freely use
    if(recycled_node_list){ //See if there exist any recycled nodes(NULL=false=empty list)
        MTF_NODE *topDeck = recycled_node_list; //Pull the pointer to the top node off the top
        recycled_node_list = topDeck->left_child; //We want to update the list(left_child as a convention)
        topDeck->left_child = 0; //Clear the left_child(every other data point is set when recycling)
        return topDeck; //We have finished removing the top node from the recycling list
    } //Since there are no recycled nodes, we want to return one from the node_pool which is by default 0s
    MTF_NODE *unusedNode = &(*(node_pool+first_unused_node_index++)); //Allocate the node at the current index
    unusedNode->left_child = 0; //just some paranoia, but setting the symbol is important
    unusedNode->right_child = 0; //just some paranoia, but this allows us to reset first_unused_index
    unusedNode->parent = 0; //just some paranoia, but this allows for multiple calls to mtf_map_encode/decode
    unusedNode->left_count = 0; //just some paranoia, but we won't run out of nodes since each node is new
    unusedNode->right_count = 0; //just some paranoia, but yea im paranoid
    unusedNode->symbol = NO_SYMBOL; //just some paranoia
    return unusedNode; //Return the node once we set the default values
}

static void recycleNode(MTF_NODE *recyclee){ //Recycle a node at address recyclee
    if(recyclee->parent->left_child == recyclee){ //Our node was the left child of the parent
        recyclee->parent->left_child = 0; //Delete our node from the parent node
    } else{ //It was the right child
        recyclee->parent->right_child = 0; //Delete our node from the parent node
    } //I won't do this for the children, since we only delete if there are no children
    recyclee->left_child = recycled_node_list; //Use left_child as a self-imposed convention
    recyclee->right_child = 0; //Clear the other fields before re-using
    recyclee->parent = 0; //Clear the other fields before re-using
    recyclee->left_count = 0; //Clear the other fields before re-using
    recyclee->right_count = 0; //Clear the other fields before re-using
    recyclee->symbol = NO_SYMBOL; //Clear the other fields before re-using(in this case; clearing is -1)
    recycled_node_list = recyclee; //Set the recyclee as the head of the list
} //I am treating recycled_node_list as a linked list where we append to the head

static MTF_NODE* descendTree(OFFSET offset){ //Given an offset, navigate to it on our tree
    MTF_NODE *leafNode = mtf_map; //Start at the head of the tree and descend the tree
    for(int i = depth-1; i >= 0; i--){ //We want to account for leading zeroes in current_offset
        int direction = ((int)(offset/getPowerOfTwo(i))&1); //Convoluted way to get the digit at bit i of offset
        if(direction){ //If direction is non-zero, it must be 1 and therefore we go right
            if(!leafNode->right_child){ //If there is no right child, we want to fix that
                leafNode->right_child = getNodePointer(); //Set the child to a node
                leafNode->right_child->parent = leafNode; //Set the parent of the new node to the old
            } //The right child has been created
            leafNode = leafNode->right_child; //Go to the right child
        } else{ //Otherwise, we go left
            if(!leafNode->left_child){ //If there is no left child, we want to fix that
                leafNode->left_child = getNodePointer(); //Set the child to a node
                leafNode->left_child->parent = leafNode; //Set the parent of the new node to the old
            } //The left child has been created
            leafNode = leafNode->left_child; //Go to the left child
        } //We have descended one level in the tree, even if there was NULL
    } //We have descended to the leaf node
    return leafNode; //Return the leaf node after desceding the tree
}

static MTF_NODE* descendTreeAndAdd(OFFSET offset){ //Given an offset, navigate to it on our tree to add
    MTF_NODE *leafNode = mtf_map; //Start at the head of the tree and descend the tree
    for(int i = depth-1; i >= 0; i--){ //We want to account for leading zeroes in current_offset
        int direction = ((int)(offset/getPowerOfTwo(i))&1); //Convoluted way to get the digit at bit i
        if(direction){ //If direction is non-zero, it must be 1 and therefore we go right
            if(!leafNode->right_child){ //If there is no right child, we want to fix that
                leafNode->right_child = getNodePointer(); //Set the child to a node
                leafNode->right_child->parent = leafNode; //Set the parent of the new node to the old
            } //The right child has been created
            leafNode->right_count++; //We are inserting a node on the right path
            leafNode = leafNode->right_child; //Go to the right child
        } else{ //Otherwise, we go left
            if(!leafNode->left_child){ //If there is no left child, we want to fix that
                leafNode->left_child = getNodePointer(); //Set the child to a node
                leafNode->left_child->parent = leafNode; //Set the parent of the new node to the old
            } //The left child has been created
            leafNode->left_count++; //We are inserting a node on the left path
            leafNode = leafNode->left_child; //Go to the left child
        } //We have descended one level in the tree, even if there was NULL
    } //We have descended to the leaf node
    return leafNode; //Return the leaf node after desceding the tree
}

static MTF_NODE* descendTreeWithRank(CODE rank){ //Given a proper rank, descend to the node with that rank
    MTF_NODE *currentNode = mtf_map; //Start at the top node
    int level = depth; //We want to go down the tree depth amount of times
    while(level>0){ //We want to traverse until we reach the bottom level
        if((currentNode->right_count-rank)>0){ //If the rank can be found on the right side
            currentNode=currentNode->right_child;
        } else{ //Otherwise, it must be on the left side
            rank-=currentNode->right_count; //We subtract the count from the right side to keep an accurate count
            currentNode=currentNode->left_child;
        } //We moved down one level
        level--; //What that comment said
    } //Note that if we are descending with the rank, it must exist in the tree already(and therefore not need to create)
    return currentNode; //We have reached the node with rank 'rank'
}

static CODE ascendTreeAndDelete(MTF_NODE *leafNode) { //Deletes the leaf and any childless nodes
    CODE rank = 0; //Start a count for the rank
    while(leafNode->parent) { //We want to keep going until we reach mtf_map, whose parent is NULL
        MTF_NODE *parent = leafNode->parent; //Save the parent of the current leaf node
        if(parent->left_child == leafNode){ //See if our current node is the left_child
            rank+=parent->right_count; //If it was, we want to add all the nodes on the right
            parent->left_count--; //The bottom-most leafNode is deleted, so the count goes down by 1
        } else {//If our node was the right_child, we WERE the right_count, so we would add 0(no change)
            parent->right_count--; //However, we still want to update the count
        } //The count for every node on the way up should be affected, but not any not on the way
        if(!leafNode->left_child && !leafNode->right_child){ //If the node has children, the OR will pick it up
            recycleNode(leafNode); //Delete the child-less leaf node by tossing it into recycle
        } //If the node wasn't a leaf, we should keep ascending without deletion
        leafNode = parent; //Set the leaf to the previous leaf's parent
    }
    return rank; //Return the accumulated rank
}

static void fibonacciCode(CODE num){ //Given a number, we want to encode it and change codeNum/codeLen
    int fibNum = 1; //Start by appending the 1 to the end
    int fibLen = 1; //Start with a length of 1(This is because fibNum is initialized)
    int fib1 = 1; //Start by creating variables to get the fibonacci numbers
    int fib2 = 1; //Start by creating variables to get the fibonacci numbers
    int temp = 1; //Create a temp variable with any value
    int flag = 1; //This will let us know if we are allowed to append a 1(1=yes,0=no)
    while(fib2<=num){ //We go until we find a fib2 is larger than num(we will iterate until fib2 exceeds num)
        temp = fib1 + fib2; //Add the two previous fibonacci numbers
        fib1 = fib2; //Move the numbers up
        fib2 = temp; //Move the numbers up
    } //Now, fib1 is the largest fibonacci number that fits within num
    fibNum+=1*getPowerOfTwo(fibLen); //Add a 1 at the current opening
    fibLen++; //We want to append the next bit further up
    flag = 0; //Fix the flag so we know not to add a 1(This catches any repeating 1s outside of the end)
    num-=fib1; //Decrease the number so we can greedily add more fibonacci numbers
    while(fib1>1){ //We want to go until fib1 reaches the fibonacci number 1, where it will go once to check
        temp = fib2; //This is the latest fibonacci number
        fib2 = fib1; //This is the one before that
        fib1 = temp-fib2; //We can reverse the fibonacci process
        if(flag && num>=fib1){ //We can fit a fibonacci number AND are allowed to append
            fibNum+=1*getPowerOfTwo(fibLen); //The 1 is unneeded but emphasizes the point
            num-=fib1; //We are greedily consuming fibonacci numbers
            fibLen++; //Increase the length so we append further up next time
            flag = 0; //We cannnot append again
        } else{ //We just added a 1 for a fibonacci number
            fibLen++; //Increase the length so we append further up next time
            flag = 1; //We are allowed to append next time//We can still append next time
        } //We have sucessfully checked that fibonacci number
    } //We finished our encoding
    codeLen+=fibLen; //Add the length of the code to the encoded length
    codeNum<<=fibLen; //Make way for the encoded number
    codeNum|=fibNum; //Append our encoded number to the code
}

static CODE fibonacciDecode(long long int fibCode, int fibLen){ //Given a zeckendorf encoding, return the encoded value
    CODE sum = 0; //We want to accumulate the number from the code
    int fib1 = 1; //We want to encode the fibonacci numbers
    int fib2 = 1; //We will use fib2 as the number
    int temp = 1; //Temp variable with any value but
    fibLen--; //2^0 is the (1)nes place, so we want to correct for that
    while(fibLen>0){ //We want to add up the ith fibonacci number for all the digits in fibCode
        sum+=fib2*((int)(fibCode/getPowerOfTwo(fibLen))&1); //Add the number, if there is a 1 present
        fibLen--; //Decrease the length until we reach 0
        temp=fib1; //Save fib1
        fib1=fib2; //Replace fib1 with fib2
        fib2=fib1+temp; //Add fib1 and fib2 in fib2
    } //We completed the encoding
    return sum; //Return the sum
}

/**
 * @brief  Given a symbol value, determine its encoding (i.e. its current rank).
 * @details  Given a symbol va`lue, determine its encoding (i.e. its current rank)
 * according to the "move-to-front" transform described in the assignment
 * handout, and update the state of the "move-to-front" data structures
 * appropriately.
 *
 * @param sym  An integer value, which is assumed to be in the range
 * [0, 255], [0, 65535], depending on whether the
 * value of the BYTES field in the global_options variable is 1 or 2.
 *
 * @return  An integer value in the range [0, 511] or [0, 131071]
 * (depending on the value of the BYTES field in the global_options variable),
 * which is the rank currently assigned to the argument symbol.
 * The first time a symbol is encountered, it is assigned
 * a default rank computed by adding 256 or 65536 to its value.
 * A symbol that has already been encountered is assigned a rank in the
 * range [0, 255], [0, 65535], according to how recently it has occurred
 * in the input compared to other symbols that have already been seen.
 * For example, if this function is called twice in succession
 * with the same value for the sym parameter, then the second time the
 * function is called the value returned will be 0 because the symbol will
 * have been "moved to the front" on the previous call.
 *
 * @modifies  The state of the "move-to-front" data structures.
 */
CODE mtf_map_encode(SYMBOL sym) {
    if(global_options & 1){ //Check the case of encoding 1 byte
        if(sym<0 || sym>255){ //If the number is negative or greater than 1 byte
            exit(EXIT_FAILURE); //The input was invalid; exit because we can't differentiate return values
        } //Now, the input is valid
    } else{ //Check the case of encoding 2 bytes
        if(sym<0 || sym>65535){ //If the number is negative or greater than 2 byte
            exit(EXIT_FAILURE); //The input was invalid; exit because we can't differentiate return values
        } //Now, the input is valid
    } //idk, this is for robustness or w.e
    if(!initialized){ //Initialize the variables if we have not done so before
        for(int i=0; i<SYMBOL_MAX; i++){ //We want to loop through for every value in last_offset
            *(last_offset+i)=NO_OFFSET; //Get the value at each index and set it to a default NO_OFFSET
        } //last_offset is initialized
        first_unused_node_index = current_offset = depth = 0; //Just paranoid, I guess
        recycled_node_list = NULL; //I know it said it would be NULL but I hope this paranoia pays off
        powerOfTwo = 1; //When current_offset reaches 1(and every subsequent powOf2), we will increase the depth
        mtf_map = getNodePointer(); //Allocate the map with a node
        initialized++; //We shall never set foot here again
    } //Let it be so
    if(current_offset == powerOfTwo){ //When current_offset reaches a power of two, the tree is full
        powerOfTwo*=2; //Simpler than calculating everytime to check when the tree is full
        depth++; //The tree has one more layer of children to deal with
        MTF_NODE *parent = getNodePointer(); //Get a pointer to be the new parent node
        mtf_map->parent = parent; //Set the parent of the current head to be the parent
        parent->left_child = mtf_map; //Link the parent to the current head as the left_child
        parent->left_count = mtf_map->left_count + mtf_map->right_count; //Left_count should be the sum of counts
        if(depth==1){ //From the base case of depth=0 and our depth++;
            parent->left_count++; //The parent node was initially a leaf itself
        } //Sorry for having a comment on every line and here's the apology making it worse
        mtf_map = parent; //Set the parent as the new head of the tree
    } //We have successfully create more space to fit the current node
    CODE rank; //I think CODE is the proper typedef for rank but honestly as long as it's int, it's good
    if(*(last_offset+sym)==NO_OFFSET){ //We have not seen this symbol yet so we have to add an offset
        if(global_options & 1){ //The arguments must be valid and 0x1(01) would not match with 0x2(10)
            rank = sym+256; //We add 256 since that can unmistakably be deciphered as sym
        } else{ //If the flag wasn't 1, then it must be 2 since there are only 2 valid arguments
            rank = sym+65536; //To elaborate on why we add, the first 65536 are for "seen before rank"
        } //We have gotten the rank of the symbol
    } else{ //We found the offset for the last time we used the symbol;
        MTF_NODE *leafNode = descendTree(*(last_offset+sym)); //Descend the tree to where sym was last seen
        rank = ascendTreeAndDelete(leafNode); //Deletes unnecessary nodes and counts the rank while doing so
    } //We got the rank of the symbol and removed the node it was last seen at
    if(depth == 0){ //Base case of building the tree; will only occur once
        mtf_map->symbol = sym; //We only need to assign the symbol for the base case
    } else{ //Now, we can assume that the tree has a proper parent node so we can navigate with 0/1
        MTF_NODE *leafNode = descendTreeAndAdd(current_offset); //Add the node to the tree and increase count
        leafNode->symbol = sym; //Set the symbol equal to the current symbol
    } //We will now have added the node in all cases
    *(last_offset+sym) = current_offset; //The place our symbol can be found is at the current_offset
    current_offset++; //Move onto the next offset so we can insert more symbols
    return rank; //Returns the rank of the symbol sym
}

/**
 * @brief Given an integer code, return the symbol currently having that code.
 * @details  Given an integer code, interpret the code as a rank, find the symbol
 * currently having that rank according to the "move-to-front" transform
 * described in the assignment handout, and update the state of the
 * "move-to-front" data structures appropriately.
 *
 * @param code  An integer value, which is assumed to be in the range
 * [0, 511] or [0, 131071], depending on the value of the BYTES field in
 * the global_options variable.
 *
 * @return  An integer value in the range [0, 255] or [0, 65535]
 * (depending on value of the BYTES field in the global_options variable),
 * which is the symbol having the specified argument value as its current rank.
 * Argument values in the upper halves of the respective input ranges will be
 * regarded as the default ranks of symbols that have not yet been encountered,
 * and the corresponding symbol value will be determined by subtracting 256 or
 * 65536, respectively.  Argument values in the lower halves of the respective
 * input ranges will be regarded as the current ranks of symbols that
 * have already been seen, and the corresponding symbol value will be
 * determined in accordance with the move-to-front transform.
 *
 * @modifies  The state of the "move-to-front" data structures.
 */
SYMBOL mtf_map_decode(CODE code) {
    if(global_options & 1){ //Check the case of encoding 1 byte
        if(code<0 || code>511){ //Check if the code is out of bounds
            return NO_SYMBOL; //It was, so we want to return -1;
        } //We cleared the errors
    } else{ //Check the case of encoding 2 bytes
        if(code<0 || code>131071){ //Check if the code is out of bounds
            return NO_SYMBOL; //It was, so we want to return -1;
        } //We cleared the errors
    } //idk, this is for robustness or w.e
    if(!initialized){ //Initialize the variables if we have not done so before
        if(global_options & 1){ //Check the case of encoding 1 byte
            if(code<=255){ //Check if the code is in the lower half
                return NO_SYMBOL; //We should not have seen any symbol yet(since it's the FIRST code)
            } //This is enough to check for the base case of uninitialized
        } else{ //Check the case of encoding 2 bytes
            if(code<=65535){ //Check if the code is in the lower half
                return NO_SYMBOL; //We should not have seen any symbol yet(since it's the FIRST code)
            } //This is enough to check for the base case of uninitialized
        } //We now verified the first code is valid
        first_unused_node_index = current_offset = depth = 0; //Just paranoid, I guess
        recycled_node_list = NULL; //I know it said it would be NULL but I hope this paranoia pays off
        powerOfTwo = 1; //When current_offset reaches 1(and every subsequent powOf2), we will increase the depth
        maxRank = 0; //Set the max allowable rank to be 0 since we have the 0
        mtf_map = getNodePointer(); //Allocate the map with a node
        initialized++; //We shall never set foot here again
    } //A lot of this code is similar to mtf_map_encode because we are rebuilding the tree
    if(global_options & 1){ //Check the case of encoding 1 byte
        if(code>maxRank && code<256){ //Check if the code is in the lower half and greater than maxRank
            return NO_SYMBOL; //The code is trying to access ranks that are not possible with our given tree
        } //This won't guarentee our decode makes sense, but it will weed out impossible encodes
    } else{ //Check the case of encoding 2 bytes
        if(code>maxRank && code<65536){ //Check if the code is in the lower half and greater than maxRank
            return NO_SYMBOL; //The code is trying to access ranks that are not possible with our given tree
        } //This won't guarentee our decode makes sense, but it will weed out impossible encodes
    } //We now verified the code is both in bounds, and not invalid
    if(current_offset == powerOfTwo){ //When current_offset reaches a power of two, the tree is full
        powerOfTwo*=2; //Simpler than calculating everytime to check when the tree is full
        depth++; //The tree has one more layer of children to deal with
        MTF_NODE *parent = getNodePointer(); //Get a pointer to be the new parent node
        mtf_map->parent = parent; //Set the parent of the current head to be the parent
        parent->left_child = mtf_map; //Link the parent to the current head as the left_child
        parent->left_count = mtf_map->left_count + mtf_map->right_count; //Left_count should be the sum of counts
        if(depth==1){ //From the base case of depth=0 and our depth++;
            parent->left_count++; //The parent node was initially a leaf itself
        } //Sorry for having a comment on every line and here's the apology making it worse
        mtf_map = parent; //Set the parent as the new head of the tree
    } //We have successfully create more space to fit the current node
    SYMBOL sym; //We want to return the proper symbol at the end
    if(global_options & 1){ //Check the case of encoding 1 byte
        if(code>255){ //Check if the code is in the upper half(never seen before)
            sym = code-256; //We want to adjust so it gets the corect symbol
            maxRank++; //We encountered a new symbol, so the maximum rank is increased
        } else{ //It must be in the lower half, so we are replacing an old symbol
            MTF_NODE *leafNode = descendTreeWithRank(code); //Descend the tree with the current rank
            sym = leafNode->symbol; //Get the symbol at the leaf node
            ascendTreeAndDelete(leafNode); //We don't need to store the rank now
        } //We have checked the code
    } else{ //Check the case of encoding 2 bytes
        if(code>65535){ //Check if the code is in the upper half(never seen before)
            sym = code-65536; //We want to adjust so it gets the corect symbol
            maxRank++; //We encountered a new symbol, so the maximum rank is increased
        } else{ //It must be in the lower half, so we are replacing an old symbol
            MTF_NODE *leafNode = descendTreeWithRank(code); //Descend the tree with the current rank
            sym = leafNode->symbol; //Get the symbol at the leaf node
            ascendTreeAndDelete(leafNode); //We don't need to store the rank now
        } //We have checked the code
    } //We obtained the symbol, now we want to build the tree, so it stays consistent for future calls
    if(depth == 0){ //Base case of building the tree; will only occur once
        mtf_map->symbol = sym; //We only need to assign the symbol for the base case
    } else{ //Now, we can assume that the tree has a proper parent node so we can navigate with 0/1
        MTF_NODE *leafNode = descendTreeAndAdd(current_offset); //Add the node to the tree and increase count
        leafNode->symbol = sym; //Set the symbol equal to the current symbol
    } //We will now have added the node in all cases
    current_offset++; //Move onto the next offset so we can rebuild the tree
    return sym; //We sucessfully made it through the program with no errors
}

/**
 * @brief  Perform data compression.
 * @details  Read uncompressed data from stdin and write the corresponding
 * compressed data to stdout, using the "move-to-front" transform and
 * Fibonacci coding, as described in the assignment handout.
 *
 * Data is read byte-by-byte from stdin, and each group of one or two
 * bytes is interpreted as a single "symbol" (according to whether
 * the BYTES field of the global_options variable is 1 or 2).
 * Multi-byte symbols are constructed according to "big-endian" byte order:
 * the first byte read is used as the most-significant byte of the symbol
 * and the last byte becomes the least-significant byte.  The "move-to-front"
 * transform is used to map each symbol read to its current rank.
 * As described in the assignment handout, the range of possible ranks is
 * twice the size of the input alphabet.  For example, 1-byte input symbols
 * have values in the range [0, 255] and their ranks have values in the
 * range [0, 511].  Ranks in the lower range are used for symbols that have
 * already been encountered in the input, and ranks in the upper range
 * serve as default ranks for symbols that have not yet been seen.
 *
 * Once a symbol has been mapped to a rank r, Fibonacci coding is applied
 * to the value r+1 (which will therefore always be a positive integer)
 * to obtain a corresponding code word (conceptually, a sequence of bits).
 * The successive code words obtained as each symbol is read are concatenated
 * and the resulting bit string is blocked into 8-bit bytes (according to
 * "big-endian" bit order).  These 8-bit bytes are written successively to
 * stdout.
 *
 * When the end of input is reached, any partial output byte is "padded"
 * with 0 bits in the least-signficant positions to reach a full 8-bit
 * byte, which is emitted as the final byte in the output.
 *
 * Note that the transformation performed by this function is to be done
 * in an "online" fashion: output is produced incrementally as input is read,
 * without having to read the entire input first.
 * Note also that this function does *not* attempt to "close" its input
 * or output streams.
 *
 * @return 0 if the operation completes without error, -1 otherwise.
 */
int mtf_encode() {
    initialized = 0; //I know it's static but come on its one line of code man
    codeLen = codeNum = 0; //Separate but these variables are not needed in mtf_map_encode
    if(global_options & 1){ //The arguments must be valid and 0x1(01) would not match with 0x2(10)
        SYMBOL input = getchar(); //Read one character at a time
        while(input!=EOF){ //Read until we get the EOF notification
            CODE code = mtf_map_encode(input); //Encode the input and store the rank
            input = getchar(); //Move to the next character to read
            fibonacciCode(code+1); //Encode the rank+1 to ensure positive numbers
            while(codeLen >= 8){ //Print out the characters 8 bits at a time
                long long int mask = 0xff; //This is binary for 11111111, used to mask 8 bits
                mask<<=(codeLen-8); //Shift the masking bits to cover the first 8 bits
                long long int masked = (codeNum&mask); //Mask the bits to get the top 8 bits
                masked>>=(codeLen-8); //Shift the masked bits back as the 8 LSBs
                if(putchar(masked)==EOF){ //Put the char out, checking if we got an error
                    return -1; //If we did, we want to return saying we did
                } //Otherwise, continue as normal
                codeLen-=8; //We read 8 bytes so the number is 8 bits less
            } //Separated the function to increase modularity or something
        } //We read the symbol and it was the EOF(-1) so we don't need to read more
    } else{ //This means that our global flag must have wanted to read two bytes at a time
        SYMBOL input1 = getchar(); //Get the first symbol
        SYMBOL input2 = getchar(); //Get the second symbol
        while(input1!=EOF && input2!=EOF){ //If any of the inputs return EOF, we want to stop encoding
            CODE code = mtf_map_encode(input1<<8 | input2); //Combine the symbols and encode t hem
            input1 = getchar(); //Renew the first symbol
            input2 = getchar(); //Renew the second symbol
            fibonacciCode(code+1); //Encode the rank+1 to ensure positive numbers
            while(codeLen >= 8){ //Print out the characters 8 bits at a time
                long long int mask = 0xff; //This is binary for 11111111, used to mask 8 bits
                mask<<=(codeLen-8); //Shift the masking bits to cover the first 8 bits
                long long int masked = (codeNum&mask); //Mask the bits to get the top 8 bits
                masked>>=(codeLen-8); //Shift the masked bits back as the 8 LSBs
                if(putchar(masked)==EOF){ //Put the char out, checking if we got an error
                    return -1; //If we did, we want to return saying we did
                } //Otherwise, continue as normal
                codeLen-=8; //We read 8 bytes so the number is 8 bits less
            } //Separated the function to increase modularity or something
        } //We finished reading from stdin
        if(input1!=EOF && input2==EOF){ //If the first input was not EOF, but the second was, we have odd bytes
            return -1; //We don't want to deal with an odd number, so we return an error after outputting n-1 bytes
        } //We don't care that if the first input was EOF, because we just wanted to stop reading then
    } //We finished reading the characters from stdin
    if(codeLen<8 && codeLen>0){ //If we have bits left to read(double check <8), we must pad it to print
        //pad lsb bits of fibNum wiht 0 if we don't have enough after encoding everything
        long long int mask = 0xff; //This is binary for 11111111, used to mask 8 bits
        codeNum<<=(8-codeLen);
        codeNum&=mask; //We don't care what's left in codeNum other than the last bits we padded
        if(putchar(codeNum)==EOF){ //Put the final byte out, checking if we got an error
            return -1; //If we did, we want to return saying we did
        } //Otherwise, we are basically done
    } //We have no more bits to print out
    if(fflush(stdout)==EOF){ //To be completely honest, not really sure what this does
        return -1; //But it's part of a 'robust program' and I get the gist of putting at the end
    } //It allows us to flush the buffer, so we can get input in order idk
    return 0; //We went through the program successfully without any error
}

/**
 * @brief  Perform data decompression, inverting the transformation performed
 * by mtf_encode().
 * @details Read compressed data from stdin and write the corresponding
 * uncompressed data to stdout, inverting the transformation performed by
 * mtf_encode().
 *
 * Data is read byte-by-byte from stdin and is parsed into individual
 * Fibonacci code words, using the fact that two consecutive '1' bits can
 * occur only at the end of a code word.  The terminating '1' bits are
 * discarded and the remaining bits are interpreted as describing the set
 * of terms in the Zeckendorf sum representing a positive integer.
 * The value of the sum is computed, and one is subtracted from it to
 * recover a rank.  Ranks in the upper half of the range of possible values
 * are interpreted as the default ranks of symbols that have not yet been
 * seen, and ranks in the lower half of the range are interpreted as ranks
 * of symbols that have previously been seen.  Using this interpretation,
 * together with the ranking information maintained by the "move-to-front"
 * heuristic, the rank is decoded to obtain a symbol value.  Each symbol
 * value is output as a sequence of one or two bytes (using "big-endian" byte
 * order), according to the value of the BYTES field in the global_options
 * variable.
 *
 * Any 0 bits that occur as padding after the last code word are discarded
 * and do not contribute to the output.
 *
 * Note that (as for mtf_encode()) the transformation performed by this
 * function is to be done in an "online" fashion: the entire input need not
 * (and should not) be read before output is produced.
 * Note also that this function does *not* attempt to "close" its input
 * or output streams.
 *
 * @return 0 if the operation completes without error, -1 otherwise.
 */
int mtf_decode() {
    initialized = 0; //Declare that we are uninitialized(I know it's static)
    codeLen = codeNum = 0; //Separate but these variables are not needed in mtf_map_encode
    int byte = getchar(); //Read the first byte of the encoding(1/2 bytes only change how the mtf_map_decode process)
    while (byte != EOF){ //Read until we read the EOF
        int nextByte = getchar(); //We want to get a taste of the next byte
        if(nextByte==EOF){ //If our current byte is the last one, we want to remove padding zeros
            int newLen = 8; //Start at 8 and decrease until we reach a 1
            while(!((int)(byte/getPowerOfTwo(0))&1)){ //Remove the padding zero
                byte>>=1; //Move one bit the LSB of the byte is a 1
                newLen--; //This section will not run if there were no padding 0
            } //There are potential bad input with padding 1, but I assume it may also be valid input so
            codeNum<<=newLen; //Move the current codeNum by the length bits to make room for our input
            codeLen+=newLen; //The length of our number is increased by newLen bits
            codeNum|=byte; //Append our input into the codeNum
        } else{ //The general case assumes we are reading all 8 bits
            codeNum<<=8; //Move the current codeNum by 8 bits to make room for our input
            codeLen+=8; //The length of our number is increased by 8 bits
            codeNum|=byte; //Append our input into the codeNum
        } //This is hopefully the last comment because honestly same
        int searchRepeat = 1; //We want to read until we read without finding repeating 1s(allows cases like 011011)
        int flag = 0; //We have not read a 1 yet(We will find repeating 1s this way)
        while(searchRepeat){ //We go until we iterate without finding a repeated digit
            searchRepeat = 0; //Assume this is the last time, unless proven otherwise
            int bits = 8; //We want to read the latest 8 bits
            if(codeLen<8){ //However, if we don't have 8 bits to read
                bits = codeLen; //We'll just read the latest bits
            } //This catches some edge cases, I hope
            for(int i = bits; i >= 0; i--){ //We want to read the latest bits for repeating 0s(Previous 11s are removed)
                int digit = ((int)(codeNum/getPowerOfTwo(i))&1); //Get the digit of codeNum at bit i(Won't affect codeNum)
                if(digit){ //The digit was a 1
                    if(flag){ //We previously read a 1
                        flag = 0; //Set the flag to 0 for later use
                        searchRepeat = 1; //We confirmed that we should loop again since we found a repeat
                        long long int codeDupe = codeNum; //Duplicate codeNum for manipulation
                        codeDupe>>=i; //We want to erase the following bits after our repeating 1
                        int dupeLen = codeLen-i; //The first couple of bits are part of the new number(don't go over)
                        codeLen=i; //We have i bits remaining after the repeating 1s

unsigned mask;
mask = (1 << i) - 1;
codeNum = codeNum & mask;

                        CODE result = fibonacciDecode(codeDupe,dupeLen); //Decode the zeckendorf encoding
                        result--; //Subtract 1 to account for the 1 we added for encoding(to ensure positive numbers
                        SYMBOL sym = mtf_map_decode(result);
                        if(sym==NO_SYMBOL){ //If we got an error message
                            return -1; //We want to let the main program know that the input broke somewhere
                        } //We finished decoding this byte
                        if(global_options & 1){ //Check the case for 1 byte
                            if(putchar(sym)==EOF){ //Put the char out, checking if we got an error
                                return -1; //If we did, we want to return saying we did
                            } //Otherwise, continue as normal
                        } else{ //Check the case for 2 bytes
                            SYMBOL sym1 = ((0xff<<8)&sym)>>8; //We want to get the first 8 bits
                            SYMBOL sym2 = 0xff&sym; //We want to get the last 8 bits
                            if(putchar(sym1)==EOF){ //Put the char out, checking if we got an error
                                return -1; //If we did, we want to return saying we did
                            } //Otherwise, continue as normal
                            if(putchar(sym2)==EOF){ //Put the char out, checking if we got an error
                                return -1; //If we did, we want to return saying we did
                            } //Otherwise, continue as normal
                        } //We have finished printing after we decoded the signal
                    } else{ //Otherwise, trigger the flag
                        flag = 1; //Next time, if we read a 1, flag will be triggered
                    } //Dealt with the 1
                } else{ //We read a 0
                    flag = 0; //The last bit we read wasn't a 1
                } //Dealt with the 0
            } //Read the bits, and dealt with any 11s that occurred
            searchRepeat = 0; //We confirmed that we shouldn't loop again since we didn't find a repeat
        } //We've finished searching for repeating 1's
        byte=nextByte; //Set the byte equal to the next byte
    } //We have finished reading the file, since we read every byte(since writing 4 bits would count as 4 leading 0 bit)
    if(codeLen>0){ //There is a case of no ending 11, which implies that the file was not proper input
        return -1; //This is invalid input (somehow messed up the input)
    } //We have dealt with a possible error
    if(fflush(stdout)==EOF){ //To be completely honest, not really sure what this does
        return -1; //But it's part of a 'robust program' and I get the gist of putting at the end
    } //It allows us to flush the buffer, so we can get input in order idk
    return 0; //We made it through the program without crashing
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */

int validargs(int argc, char **argv) {
    int global_options_save = global_options; //Save the value of global_options if it's not 0x0(piazza)
    global_options <<= 3; //Realizes that global_options is auto storage so it will always be 0, but it works
    global_options = (unsigned)global_options>>3; //Cast it to be unsigned so we get logical shift right
    if(argc==1){ //We want to make sure that there is a flag provided
        return -1; //The only argument passed was the filename
    } //Note: 0 args implies no -h AND none of -e/-d(none!=exactly one)
    argv++; //Skips file name(zeroth argument). I didn't use switch/case since 4 cases doesn't speed it up
    if(strEquals((*argv), "-h")){ //Check if the help flag is enabled. Could have ++(*argv) to save a line
        global_options |= HELP_OPTION; //Do bitwise OR on global_options to set the bit
        return 0; //We don't want to check any other arguments if -h is provided
    } //From this point on, the -h flag should not appeart(it must the first flag)
    if(strEquals((*argv), "-e")){ //Check if the encode flag is enabled
        global_options |= ENCODE_OPTION; //Do bitwise OR on global_options to set the bit
    }else if(strEquals(*argv, "-d")){ //Check if the decode flag is enabled
        global_options |= DECODE_OPTION; //Do bitwise OR on global_options to set the bit
    } else { //The byte flag cannot occur out of order, and any other options are invalid
        return -1; //The first flag isn't valid and global_options hasn't been changed yet
    } //From this point on, the only flag allowed is -b(-e/-d cannot exist with each other)
    if(argc==2){ //We only had the first flag, so we are using the default value of 1 for the byte
        global_options >>= 7; //Erase the 7 LSBs, the new MSBs can be ignored since we will shift left
        global_options <<= 7; //Re-introduce the LSBs but as 0 due to the logical left shift
        global_options |= 0x1; //The -b flag could not have existed so the byte value is 1
        return 0; //I tried to make the setting bytes as a macro, but it failed and this isn't overused
    } //From this point on, we are reading the -b flag
    argv++; //We want to check the second flag//third argument(should always be the byte flag)
    if(!strEquals((*argv), "-b")){ //We have to check that is actually is the -b flag
        global_options = global_options_save; //Restore the value if there is an error
        return -1; //The flag was invalid(-h returns, -e/-d cannot coexist and other values don't match)
    }//We now know that the flag must be -b
    if(argc!=4){ //We should have 4 arguments at this point: filename, -h/-e/-d, -b, byte specification
        global_options = global_options_save; //Restore the value if there is an error
        return -1; //If that isn't the case, the number of arguments is invalid
    } //It may be optimal to check this before checking for -b, but I think it depends on test cases
    argv++; //We want to check the parameter for the byte flag
    while((**argv)=='0'){ //We want to accept any amount of leading zeros
        (*argv)++; //Read the next character until we are out of leading zeros
    } //This means that the specification in the byte flag should have no more 0(only accept 1/2 now)
    switch((**argv)){ //I decided I should learn how to use it and I didn't want to refactor the code...
        case '1': //We want to set the last 7 bytes to 0x1
            global_options >>= 7; //Erase the 7 LSBs, the new MSBs can be ignored since we will shift left
            global_options <<= 7; //Re-introduce the LSBs but as 0 due to the logical left shift
            global_options |= 0x1; //The -b flag could not have existed so the byte value is 1
            break; //We want to make sure there are no values after here otherwise 10 could pass
        case '2': //We want to set the last 7 bytes to 0x2
            global_options >>= 7; //Erase the 7 LSBs, the new MSBs can be ignored since we will shift left
            global_options <<= 7; //Re-introduce the LSBs but as 0 due to the logical left shift
            global_options |= 0x2; //The -b flag could not have existed so the byte value is 2
            break; //We want to make sure there are no values after here otherwise 20 could pass
        default: //The argument must have been invalid
            global_options = global_options_save; //Restore the value if there is an error
            return -1; //Therefore, we should return invalid arguments
    } //From here, we want to check that there are no more characters
    (*argv)++; //This character should be '\0' or the null character
    if((**argv)!='\0'){ //If it wasn't null, we should return an error
        global_options = global_options_save; //Restore the value if there is an error
        return -1; //The artgument was invalid
    } //We couldn't use the same return style as strEquals since it would not return 0/-1 without hassle
    return 0; //All cases of invalid arguments were dealt with prior to this point
}
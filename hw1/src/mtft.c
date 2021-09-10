/*
 * MTF: Encode/decode using "move-to-front" transform and Fibonacci encoding.
 */

#include <stdlib.h>
#include <stdio.h>

#include "mtft.h"
#include "debug.h"

static int depth; //Keep track of the depth of the tree which is also how many bits the offset should be
static int powerOfTwo; //I chose this rather than a lg/ceil approach since I'm declaring a variable for depth
static int strEquals(char *str1, char *str2) { //Compare two strings and returns true if they were equal
    for(; (*str1)!='\0'; str1++,str2++){ //Iterate through str1 until we reach the null/end
        if((*str1)!=*(str2)){ //Compare the current characters in both string
            return 0; //The characters were not equal so the string must not be
        } //The current chracters of each string were equal, so we want to move on
    } //This loop catches all cases ONLY when str1 is shorter than str2
    return (*str2)=='\0'; //If str2 is longer/did not reach the end, it must not be the same string
} //Found some ways to (maybe) make the code more efficient(and harder to read but comments I guess)


//DELETE AND FIX
static SYMBOL getSymbol() { //Return the number of bytes that a symbol represents
    if(global_options & 1){ //The arguments must be valid and 0x1(01) would not match with 0x2(10)
        return getchar(); //Read one character and return it
    } else { //If the flag wasn't 1, then it must be 2 since there are only 2 valid arguments
        //return getchar()<<8 | getchar(); //Is cool but doesn't allow for checking each individual byte
        SYMBOL byte1 = getchar(); //Read the first byte
        if(byte1==-1) //We don't want to return if the byte is valid
            return byte1; // If the first character is -1, then we want to return -1
        SYMBOL byte2 = getchar(); //Read the second byte
        if(byte2==-1) //This should return an error since we have an odd number of input bytes
            return -1;
    } //Does not treat -1/end of file any differently than normal bytes
    return -1;
} //NOTE THS SHOUKLD BE DELELTED AND REDONE IN MTFENCODE
//while i have me here, initalize the values of last_offset to be NO_OFFSET



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
    // TO BE IMPLEMENTED. useless comment but looks nicer for now :)
debug("PRE SYMBOL:%c/%u",sym,sym);
debug("PRE LAST OFFSET:%li",*(last_offset+sym));
debug("PRE CURRENT OFFSET:%lu",current_offset);
//when current_offset = power_of_2 then power_of_two multiply by 2 and depth++
    //creat new parent node

    CODE rank; //I think CODE is the proper typedef for rank but honestly as long as it's int, it's good
    if(*(last_offset+sym)==NO_OFFSET){ //We have not seen this symbol yet so we have to add an offset
        if(global_options & 1){ //The arguments must be valid and 0x1(01) would not match with 0x2(10)
            rank = sym+256; //We add 256 since that can unmistakably be deciphered as sym
        } else{ //If the flag wasn't 1, then it must be 2 since there are only 2 valid arguments
            rank = sym+65536; //To elaborate on why we add, the first 65536 are for "seen before rank"
        } //We have gotten the rank of the symbol
    } else{ //We found the offset for the last time we used the symbol


        /* *(last_offset+sym) = offset for symbol
         *descend the tree to reach the leaf node with 0-left/1-right from MSB->LSB
         *now we want to determine rank
         *  ascend the tree by going to parent node
         *  delete any nodes that become leaf
         *  Also remember to keep the values of the tree consistent
         *  Use right_count to determine rank
         *      On the parent node, check left/right child
         *           If we came from left, add right_count
         *           If we came from right, add 0
         *              AKA: keep address of current, go to parent and compare with left/right child
         *HOW TO BUILD TREE ACCORDING TO TINY BRAIN - idk lol

    typedef struct mtf_node {
        struct mtf_node *left_child;   // Pointer to left child node
        struct mtf_node *right_child;  // Pointer to right child node
        struct mtf_node *parent;       // Pointer to parent node
        int left_count;                // Number of symbols stored below left child
        int right_count;               // Number of symbols stored below right child
        SYMBOL symbol;                 // Symbol stored at this node (only at a leaf)
    } MTF_NODE;

         */


    } //We got the rank of the symbol and removed the node it was last seen at


    //start work here
    //create an if/else for if tree exists
        //if(depth == 0) //aka if tree is just chillig with itself
            //set the node symbol to first bit
        //otherwise, we can do the 0/1 trock

    /*Descend the tree using our 0/1 trick
     *   If we find any nodes that point to null, we want to initialize the node
     *       Find a node from recycling (or the main pool if none exist)
     *   On the final node(aka the leaf), set the symbol to be sym
     *   Also remember to keep the values of the tree consistent
     */
    *(last_offset+sym) = current_offset; //The place our symbol can be found is at the current_offset
    current_offset++; //Move onto the next offset so we can insert more symbols

debug("RANK:%u", rank);
debug("POST SYMBOL:%c/%u",sym,sym);
debug("POST LAST OFFSET:%li",*(last_offset+sym));
debug("POST CURRENT OFFSET:%lu",current_offset);



    return rank; //Returns the rank of the symbol sym
} //return NO_SYMBOL; //default return seems to bewrong to me though

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
    // TO BE IMPLEMENTED.
    return NO_SYMBOL;
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
    for(int i=0; i<SYMBOL_MAX; i++){ //We want to loop through for every value in last_offset
        *(last_offset+i)=NO_OFFSET; //Get the value at each index and set it to a default NO_OFFSET
    }
    first_unused_node_index = current_offset = depth = 0; //Just paranoid, I guess
    powerOfTwo = 1; //when current_offset reaches 1(and every subsequent powOf2), we will increase the depth

    //initialize a node for MTF_NODE *mtf_map, so first symbol(0 offset) doesn't increase depth
        //next is offset 1(pow2) which will make depth 1(parent with 1 depth child)
        //next if offset 2(pow2) which will make depth 2(paren with 2 depth childd)
        //next if offset 4(pow2) will make deph 3(offset3 is not pow2 so 2deph good (0,1,2,3))
            //pull from MTF_NODE node_pool[MAX_NODES]
                //"probably &node_pool[0]"
                //akak &(*(node_pool+0))

    //MTF_NODE *recycled_node_list  dont' need intializing



    mtf_map_encode(getchar());
debug("\n\n\n");
    mtf_map_encode(getchar());

    return -1;
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
    // TO BE IMPLEMENTED
    return -1;
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
    global_options <<= 3; //If there isn't garbage, then it's just a some lines of code/allocated memory
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
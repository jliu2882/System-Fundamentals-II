#include <stdio.h>
#include <stdlib.h>

#include "mtft.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
There should be no additional prints to console or anything other than needed(USAGE)

*/
int main(int argc, char **argv)
{
    if(validargs(argc, argv)){ //If the argument was invalid(-1/non-zero/TRUE), we want to show the usage
        USAGE(*argv, EXIT_FAILURE); //We will show the user the correct flags and return with exit failure
    } //The arguments must be valid and global_option will be set as appropiate
    if(global_options & HELP_OPTION){ //If the help option flag is enabled, we want to show the usage
        USAGE(*argv, EXIT_SUCCESS); //Since this is intended, we will return with exit success
    } //Any garbage values for the first 3 flag bits will have not been reset since args are valid
    if(global_options & ENCODE_OPTION){ //If the encode option flag is enabled, we want to run the function
        if(mtf_encode()){
            debug("error");
        } else{
            debug("funny stinky");
        }



    /*
    If the -e flag is provided, then the program will read uncompressed data from
    standard input (stdin) and emit compressed data to standard output (stdout)
        tldr run encode function in if statement
        if we get a -1(TRUE), we do a goofy looking exit(exit-failure)
        if we get a 0(false), we do a goofy exit successs kinda guy
    */
    } //Any garbage values for the first 3 flag bits will have not been reset since args are valid
    if(global_options & DECODE_OPTION){ //If the decode option flag is enabled, we want to run the function
        debug("AHHHHHHHH DECODE ME\n");
    /*
    If the -d flag is provided, then the program will read compressed data from
    standard input (stdin) and emit uncompressed data to standard output (stdout).
    */
    } //Any garbage values for the first 3 flag bits will have not been reset since args are valid

    // TO BE IMPLEMENTED
    return EXIT_FAILURE; //This implies that the encode/decode functions ran into an error
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */

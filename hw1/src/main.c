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
can call exit() anywhere
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
        if(mtf_encode()){ //If the program ran into any error
            return EXIT_FAILURE; //We want to let the program know we failed
        } //Otherwise, we succesfully ran mtf_encode
    } //Any garbage values for the first 3 flag bits will have not been reset since args are valid
    if(global_options & DECODE_OPTION){ //If the decode option flag is enabled, we want to run the function
        if(mtf_decode()){ //If the program ran into any error
            return EXIT_FAILURE; //We want to let the program know we failed
        } //Otherwise, we succesfully ran mtf_encode
    } //Any garbage values for the first 3 flag bits will have not been reset since args are valid
    return EXIT_SUCCESS; //This implies that we ran our program without running into an error
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */

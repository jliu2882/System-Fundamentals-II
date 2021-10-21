#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "sfmm.h"
#include "debug.h"

int main(int argc, char const *argv[]) {

    for(int i = 0; i < 100; i++){
        char *x = sf_malloc(1);
        char *y = sf_malloc(1);
        if(i%3==0)
            sf_free(x);
        if(i%2==0)
            sf_free(y);
    }


    sf_show_heap();

    return EXIT_SUCCESS;
}


/*
 * File system browser.
 * E. Stark
 * 11/3/93
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h> /* getopt */
#include <string.h>
#include "browse.h"

int main(int argc, char *argv[]) //Could have made a usage function but it is what it is
{
  int err; //Declares variable err
  char *getcwd(), *base; //Declares the function getcwd() as well as a string base
  extern int humanReadable; //get the global vars
  extern int sortBy; //get the global vars
  int humanReadableFlag = 0; //set a flag to see if we've seen it before
  int sortFlag = 0; //set a flag to know if we've chosen a sorting option yet
  int chdirFlag = 0; //we don't want to be able to put two arguments for moving directories
  humanReadable = 0; //initialize to 0
  sortBy = 4; //sort by none by default
  int c;
  while (1){
    int option_index = 0;
    static struct option long_options[] =
    {
        {"sort-key",        required_argument, NULL,          's'},
        {"human-readable",  no_argument,       &humanReadable,  1},
        {NULL,              0,                 NULL,            0}
    };
    c = getopt_long(argc, argv, "-:s:", long_options, &option_index); //get the next argument
    if (c == -1) //if we are out of args
      break; //break the loop
    switch (c){ //we are to accept the behaviour of getopt_long, so these cases are enough
      case 0: //only one long option since sort-key is considered -s
        if(humanReadableFlag){ //this would occur if we tried to double the arguments
          fprintf(stderr, "USAGE: %s [dir] [--sort-key={name|date|size|none}] [--human-readable]\n" \
          "   dir\n" \
          "         dir must be a directory we can reach from %s\n\n" \
          "   -s, --sort-key {name|date|size|none}\n" \
          "         Sort the files and subdirectories when you open a directory\n\n" \
          "   --human-readable\n" \
          "         File sizes are displayed in 'human readable' form rather than in bytes\n\n" \
          "Note that the arguments can occur out of order. However, duplicate arguments are not allowed.\n"
          , argv[0], argv[0]); //Lets the user know how to run the program; note: update other 3 usag
          exit(EXIT_FAILURE); //Exits the program with an error
        }
        humanReadableFlag = 1; //Set the flag so we know not to do it again
        break; //triggered the flag and set the global var, so we're good
      case 's': //sort-key, allows for --sort, etc..
        if(sortFlag){ //this is triggered if we've chosen an option
          fprintf(stderr, "USAGE: %s [dir] [--sort-key={name|date|size|none}] [--human-readable]\n" \
          "   dir\n" \
          "         dir must be a directory we can reach from %s\n\n" \
          "   -s, --sort-key {name|date|size|none}\n" \
          "         Sort the files and subdirectories when you open a directory\n\n" \
          "   --human-readable\n" \
          "         File sizes are displayed in 'human readable' form rather than in bytes\n\n" \
          "Note that the arguments can occur out of order. However, duplicate arguments are not allowed.\n"
          , argv[0], argv[0]); //Lets the user know how to run the program; note: update other 3 usag
          exit(EXIT_FAILURE); //Exits the program with an error
        }
        sortFlag=1; //set a flag so we don't choose a new option
        if(strcmp(optarg,"name")==0){ //strcasecmp if that's needed
          sortBy = 1; //arbitrarily choose 1 as name
        } else if(strcmp(optarg,"size")==0){
          sortBy = 2; //arbitrarily choose 2 as size
        } else if(strcmp(optarg,"date")==0){
          sortBy = 3; //arbitrarily choose 3 as date
        } else if(strcmp(optarg,"none")==0){
          sortBy = 4; //arbitrarily choose 4 as none; already set but just for completeness
        } else{
          fprintf(stderr, "USAGE: %s [dir] [--sort-key={name|date|size|none}] [--human-readable]\n" \
          "   dir\n" \
          "         dir must be a directory we can reach from %s\n\n" \
          "   -s, --sort-key {name|date|size|none}\n" \
          "         Sort the files and subdirectories when you open a directory\n\n" \
          "   --human-readable\n" \
          "         File sizes are displayed in 'human readable' form rather than in bytes\n\n" \
          "Note that the arguments can occur out of order. However, duplicate arguments are not allowed.\n"
          , argv[0], argv[0]); //Lets the user know how to run the program; note: update other 3 usag
          exit(EXIT_FAILURE); //Exits the program with an error
        }
        break;
      case 1: //non option args, aka the directory
        if(chdirFlag){//If we've changed directories before)
          fprintf(stderr, "USAGE: %s [dir] [--sort-key={name|date|size|none}] [--human-readable]\n" \
          "   dir\n" \
          "         dir must be a directory we can reach from %s\n\n" \
          "   -s, --sort-key {name|date|size|none}\n" \
          "         Sort the files and subdirectories when you open a directory\n\n" \
          "   --human-readable\n" \
          "         File sizes are displayed in 'human readable' form rather than in bytes\n\n" \
          "Note that the arguments can occur out of order. However, duplicate arguments are not allowed.\n"
          , argv[0], argv[0]); //Lets the user know how to run the program; note: update other 3 usag
          exit(EXIT_FAILURE); //Exits the program with an error
        }
        if(chdir(optarg) < 0) { //If we cannot reach the directory with chdir, it must have been invalid
          fprintf(stderr, "Can't change directory to %s\n", optarg); //Let the user know the directory is invalid
          exit(EXIT_FAILURE); //Exits the program with an error
        } //yay
        chdirFlag = 1; //we have now changed directories
        break;
      case ':': //missing arguments
      case '?': //unknown options
      default: //catch all other bad arguments
        fprintf(stderr, "USAGE: %s [dir] [--sort-key={name|date|size|none}] [--human-readable]\n" \
          "   dir\n" \
          "         dir must be a directory we can reach from %s\n\n" \
          "   -s, --sort-key {name|date|size|none}\n" \
          "         Sort the files and subdirectories when you open a directory\n\n" \
          "   --human-readable\n" \
          "         File sizes are displayed in 'human readable' form rather than in bytes\n\n" \
          "Note that the arguments can occur out of order. However, duplicate arguments are not allowed.\n"
          , argv[0], argv[0]); //Lets the user know how to run the program; note: update other 3 usag
        exit(EXIT_FAILURE); //Exits the program with an error
    }
  } //arguments parsed and we are in the right directory
  base = getcwd(NULL, MAXPATHLEN+1); //Set the base to the current directory(both inputs may need update??)
  if((cursor_node = get_info(base)) == NULL) { //Assign our cursor_node to the information node
    fprintf(stderr, "Can't stat %s\n", base); //If we ran into an error, we should let the user know
    exit(EXIT_FAILURE); //Exits the program with an error
  }
  initdisplay(); //initializes the display
  cursor_line = 0; //Initialize cursor_line(redundant since we call in initdisplay??)
  do redisplay(); while(!(err = command(0))); //negative return = fail; otherwise = user ends

  while(cursor_node->prev != NULL && cursor_node > 0) { //if there is a previous node AND a previous line
    cursor_node = cursor_node->prev; //go to the previous node
    cursor_line--; //go to the previous linerst != N
  } //we should be on the first line/node unless there is a discrepancy
  while(cursor_node->next != NULL) { //while there is a next node
    delete_node(cursor_node); //delete the next node, and move the other nodes up
  }
  if(cursor_node->info != NULL) free(cursor_node->info); //if the deleted node had info, we want to free it
  free(cursor_node);//free the first node
  free(base); //frees memory for base

  enddisplay(); //ends the program
  exit(err < 0 ? EXIT_FAILURE : EXIT_SUCCESS); //If we ran into any errors we want to exit with failure
}

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

#include "browse.h"

int main(int argc, char *argv[])
{
  int err; //Declares variable err
  char *getcwd(), *base; //Declares the function getcwd() as well as a string base
  if(argc == 2) { //If we only have one other argument(the directory), we want to validate it
    if(chdir(argv[1]) < 0) { //If we cannot reach the directory with chdir, it must have been invalid
      fprintf(stderr, "Can't change directory to %s\n", argv[1]); //Let the user know the directory is invalid
      exit(EXIT_FAILURE); //Exits the program with an error
    } //yay
  } else if(argc != 1) { //Otherwise, we want to make sure that there are no arguments
    fprintf(stderr, "Usage: %s [dir]\n", argv[0]); //Lets the user know how to run the program
    exit(EXIT_FAILURE); //Exits the program with an error
  } //yay
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
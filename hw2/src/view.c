/*
 * "View mode", in which we view the contents of a file.
 * To keep things simple, we read in the whole file at once.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "browse.h"

void view_file(NODE *node)
{
  FILE *f; //Creates a pointer to a file
  char buf[MAXLINE+1]; //replace with infinite ??
 // char * buf = NULL;
  //size_t len = 0;
 // ssize_t read;

  int save_cursor_line = cursor_line; //saves cursor line
  NODE *save_cursor_node = cursor_node; //saves cursor node
  NODE *first, *last, *new, *next; //declares pointers yay

  if(node->info == NULL //if we have no information on the file type
     || (((node->info->stat.st_mode & S_IFMT) != S_IFREG) //or if its not a regular file
	 && ((node->info->stat.st_mode & S_IFMT) != S_IFLNK))) { //and its not a regular link
    feep("Not a regular file or link"); //let us know its not regular
    return; //return
  }
  if ((f = fopen(node->info->path, "r")) == NULL) { //if we don't have permission to read the file
    feep("Can't open file"); //let us know we cant read
    return; //return
  }
  first = last = NULL; //initialize nodes as null
  while((new = calloc(1,sizeof(NODE))) != NULL) { //while we have memory, we allocate until we break no free??
    if(fgets(buf, MAXLINE, f) == NULL) break;
 //   if((read=getline(&buf, &len, f))<=0) break;
    strncpy(new->data, buf, MAXLINE);
 //   strncpy(new->data, buf, read);

    //bootleg way to get info to add
    FILE_INFO *info; //Declares a variable to store file info
    if((info = calloc(1,sizeof(FILE_INFO))) == NULL){ //Allocate space for info ??when do we free
      feep("Out of memory"); //Lets the user know we ran out of space
      free(new); //at least try to free everything
      return; //should exit but for the sake of freeing memory I guess
    }
    new->info = info; //Sets the information about the file to be info
    new->info->level = 0;

    if(first == NULL) first = last = new;
    else last = insert_node(last, new);

 //   free(buf);
//    buf=NULL;

  }
  fclose(f); //close the file

//  if(buf!=NULL){
 //   free(buf);
 //   buf=NULL;
 // }

  if(new!=NULL){ //we only break the loop if new is null or we just allocated new and did nothing
    free(new);
  }
  if(first != NULL) {
    cursor_node = first;
    cursor_line = 0;
    do { redisplay(); } while(!command(1));
  } else {
    feep("Empty file");
    return;
  }
  while(first->next != NULL) { //while there is a next node
    next = first->next;  /* Save because delete_node frees its arg */
    delete_node(first); //delete the next node, and move the other nodes up
  }
  next = next;//temp fix lmao
  if(first->info != NULL) free(first->info); //if the deleted node had info, we want to free it
  free(first);//free the first node
  cursor_node=save_cursor_node;
  cursor_line=save_cursor_line;
 // refreshdisplay(); //clear display (REMOVE once we figure bug) I DID IT
}

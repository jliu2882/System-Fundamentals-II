
/*
 * Routines for dealing with the display list
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#include "browse.h"

int humanReadable;

static void cvt_info(FILE_INFO *info, char *buf, NODE *test);
static char *cvt_mode(mode_t mode);

/*
 * Construct an information node, given a pathname
 */

NODE *get_info(char *path)
{
  FILE_INFO *info; //Declares a variable to store file info
  NODE *node; //Declares a node to store our information

  if((info = calloc(1,sizeof(FILE_INFO))) == NULL //Allocate space for info ??when do we free
     || (node = calloc(1,sizeof(NODE))) == NULL) { //Allocate space for node
    if(info!=NULL){
      free(info); //free if we allocated
    }
    if(node!=NULL){
      free(node); //free if we allocated
    }
    feep("Out of memory"); //Lets the user know we ran out of space
    return(NULL); //Return an error if we ran out of space
  }
  info->parent = NULL; //There is no parent directory
  node->info = info; //Sets the information about the file to be info
  node->info->level = 0; //The subdirectories want 0 as indenting
  node->next = node->prev = NULL; //There are no next/previous nodes currently
  strcpy(info->path, path); //Copy our path into the information
  if(stat(path, &info->stat) < 0) { //Obtain information about our path and put it under info stat
    feep("Can't stat file"); //If we ran into any error, we want to let the user know
    free(info); //Free the space we allocated for the info
    free(node); //Free the space we allocated for the node
    return(NULL); //Return an error if we couldn't get anything about our path
  }
  cvt_info(info, node->data, node); //Store the data from info into our node
  return(node); //Returns the node with the information
}

/*
 * Convert file information to a printable line
 */

static void cvt_info(FILE_INFO *info, char *buf, NODE *test)
{
  char *n; //Declare a string n
  struct passwd *pw; //Declare a password struct(pre-defined somewhere so dw)

  if(strcmp(info->path, "/") //If our path is not "/"
     && (n = rindex(info->path, '/')) != NULL) //AND there is an occurence of "/"
      n++; //Move n to start at the next character(Skip initial "/")
  else //This is the case of no "/" or just "/"
      n = info->path; //n would just be our path
  pw = getpwuid(info->stat.st_uid); //Searches for a matching uid and returns a password

#ifdef NO_MAXLINE // I know this is bad code and I could optimize the runtime/general look but it runs man
  if(!humanReadable){ //normal mode
 //didnt work or as least didnt try to go further than this, ok nvm it worked but I did a wonky workaround
    size_t needed = snprintf(NULL, 0, "%.10s %3ld %-8.8s % 8li %.12s %s", //Write all this into buf
      cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
      info->stat.st_nlink, //The number of hard links
      pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
      info->stat.st_size, //The total size(in bytes unless specified otherwise)
      ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
      n) + 1;
    test->data = malloc(needed);

    sprintf(test->data, "%.10s %3ld %-8.8s % 8li %.12s %s", //Write all this into buf
      cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
      info->stat.st_nlink, //The number of hard links
      pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
      info->stat.st_size, //The total size(in bytes unless specified otherwise)
      ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
      n); //As well as our path
  } else{ //human readable mode //hard-coded but I didn't want to deal with allocating memory for a char*

    int size = info->stat.st_size; //get the size of the file
    if(size>pow(2,30)){ //large large files(treat as M lol)
      size_t needed = snprintf(NULL, 0, "%.10s %3ld %-8.8s % 7liM %.12s %s", //Write all this into buf
        cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
        info->stat.st_nlink, //The number of hard links
        pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
        (long int)(info->stat.st_size/pow(10,6)), //The total size(in bytes unless specified otherwise)
        ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
        n) + 1;
      test->data = malloc(needed);

      sprintf(test->data, "%.10s %3ld %-8.8s % 7liM %.12s %s", //Write all this into buf
        cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
        info->stat.st_nlink, //The number of hard links
        pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
        (long int)(info->stat.st_size/pow(10,6)), //The total size(in bytes unless specified otherwise)
        ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
        n); //As well as our path
    } else if(size>pow(2,20) && size<pow(2,30)){ //M
      size_t needed = snprintf(NULL, 0, "%.10s %3ld %-8.8s % 7liM %.12s %s", //Write all this into buf
        cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
        info->stat.st_nlink, //The number of hard links
        pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
        (long int)(info->stat.st_size/pow(10,6)), //The total size(in bytes unless specified otherwise)
        ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
        n) + 1;
      test->data = malloc(needed);

      sprintf(test->data, "%.10s %3ld %-8.8s % 7liM %.12s %s", //Write all this into buf
        cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
        info->stat.st_nlink, //The number of hard links
        pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
        (long int)(info->stat.st_size/pow(10,6)), //The total size(in bytes unless specified otherwise)
        ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
        n); //As well as our path
    } else if(size>pow(2,10) && size<pow(2,20)){ //K
      size_t needed = snprintf(NULL, 0, "%.10s %3ld %-8.8s % 7liK %.12s %s", //Write all this into buf
        cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
        info->stat.st_nlink, //The number of hard links
        pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
        (long int)(info->stat.st_size/pow(10,3)), //The total size(in bytes unless specified otherwise)
        ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
        n) + 1;
      test->data = malloc(needed);

      sprintf(test->data, "%.10s %3ld %-8.8s % 7liK %.12s %s", //Write all this into buf
        cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
        info->stat.st_nlink, //The number of hard links
        pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
        (long int)(info->stat.st_size/pow(10,3)), //The total size(in bytes unless specified otherwise)
        ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
        n); //As well as our path
    } else{ //Small files
      size_t needed = snprintf(NULL, 0, "%.10s %3ld %-8.8s % 8li %.12s %s", //Write all this into buf
        cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
        info->stat.st_nlink, //The number of hard links
        pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
        info->stat.st_size, //The total size(in bytes unless specified otherwise)
        ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
        n) + 1;
      test->data = malloc(needed);

      sprintf(test->data, "%.10s %3ld %-8.8s % 8li %.12s %s", //Write all this into buf
        cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
        info->stat.st_nlink, //The number of hard links
        pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
        info->stat.st_size, //The total size(in bytes unless specified otherwise)
        ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
        n); //As well as our path
    }
  }
#else //again i know its janky; especially since I'm doubling the already janky code
    if(!humanReadable){ //normal mode
        sprintf(buf, "%.10s %3ld %-8.8s % 8li %.12s %s", //Write all this into buf
          cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
          info->stat.st_nlink, //The number of hard links
          pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
          info->stat.st_size, //The total size(in bytes unless specified otherwise)
          ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
          n); //As well as our path
    } else{ //human readable mode //hard-coded but I didn't want to deal with allocating memory for a char*

    int size = info->stat.st_size; //get the size of the file
    if(size>pow(2,30)){ //large large files(treat as M lol)
      sprintf(buf, "%.10s %3ld %-8.8s % 7liM %.12s %s", //Write all this into buf
        cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
        info->stat.st_nlink, //The number of hard links
        pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
        (long int)(info->stat.st_size/pow(10,6)), //The total size(in bytes unless specified otherwise)
        ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
        n); //As well as our path
    } else if(size>pow(2,20) && size<pow(2,30)){ //M
      sprintf(buf, "%.10s %3ld %-8.8s % 7liM %.12s %s", //Write all this into buf
        cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
        info->stat.st_nlink, //The number of hard links
        pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
        (long int)(info->stat.st_size/pow(10,6)), //The total size(in bytes unless specified otherwise)
        ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
        n); //As well as our path
    } else if(size>pow(2,10) && size<pow(2,20)){ //K
      sprintf(buf, "%.10s %3ld %-8.8s % 7liK %.12s %s", //Write all this into buf
        cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
        info->stat.st_nlink, //The number of hard links
        pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
        (long int)(info->stat.st_size/pow(10,3)), //The total size(in bytes unless specified otherwise)
        ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
        n); //As well as our path
    } else{ //Small files
      sprintf(buf, "%.10s %3ld %-8.8s % 8li %.12s %s", //Write all this into buf
        cvt_mode(info->stat.st_mode), //Include the file type and mode in a printable format
        info->stat.st_nlink, //The number of hard links
        pw != NULL ? pw->pw_name : "", //If the password is not NULL, include it
        info->stat.st_size, //The total size(in bytes unless specified otherwise)
        ctime(&info->stat.st_mtime)+4, //The time we last accessed the file without the day of the week
        n); //As well as our path
    }
  }

#endif
}

/*
 * Convert mode bits to printable representation, a la "ls -l"
 * i hope there is no bugs in here lmao this looks spooky ??
 */

static char *cvt_mode(mode_t mode)
{
  static char buf[11];
  char *bp, c;
  int i;
  mode_t m = mode;

  bp = &buf[10];
  *bp-- = '\0';
  for(i = 0; i < 9; i++, m >>= 1) {
    switch(i%3) {
    case 0:
      /* Need to handle setuid/setgid */
      if((mode & S_ISUID && i == 6)
   || (mode & S_ISGID && i == 3)) c = 's';
      else c = 'x';
      break;
    case 1:
      c = 'w';
      break;
    case 2:
      c = 'r';
      break;
    }
    if(m & 01) *bp-- = c;
    else *bp-- = '-';
  }
  switch(mode & S_IFMT) {
  case S_IFREG:
    *bp-- = '-';
    break;
  case S_IFDIR:
    *bp-- = 'd';
    break;
  case S_IFBLK:
    *bp-- = 'b';
    break;
  case S_IFCHR:
    *bp-- = 'c';
    break;
  case S_IFLNK:
    *bp-- = 'l';
    break;
  case S_IFSOCK:
    *bp-- = 's';
    break;
  case S_IFIFO:
    *bp-- = 'f';
    break;
  }
  return(buf);

}

/*
 * Insert a new node after a given node
 */

NODE *insert_node(NODE *old, NODE *new)
{
  new->prev = old; //the new nodes past node is the old one
  new->next = old->next; //we want to insert not replace
  old->next = new; //the old nodes new node is the new node
  if(new->next != NULL) new->next->prev = new; //if the next node wasn't null, we want to set its prev to the new
  return(new); //return the new node
}
/*
 * Delete the node following a given node and free it
 */

void delete_node(NODE *node)
{
  NODE *next = node->next; //create a node for the next node that we will delete

  if(next == NULL) return; //if it was null, then we don't need to delete the next node
  node->next = next->next; //jump over the next node
  if(node->next != NULL) node->next->prev = node; //if the new next is valid, fix the previous node
  if(next->info != NULL){
    free(next->info);
  }  //if the deleted node had info, we want to free it

#ifdef NO_MAXLINE //if data is an array we don't need to free it
  if(next->data != NULL){
    free(next->data);
  }  //if the deleted node had data, we want to free it
#endif
  free(next); //free the deleted node
}


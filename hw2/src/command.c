
/*
 * Process keyboard input and dispatch commands
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <curses.h>
#include <curses.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>

#include "browse.h"

static void next_line();
static void previous_line();
static void next_screen();
static void previous_screen();
static void open_directory(NODE *dir);

static int compareNode(NODE *first, NODE* second);

/*
 * Reinsert a node into a new position
 */
NODE *reinsert_node(NODE *old, NODE *new);
//include here since we cant change browse.h and i dont want to include info.c

/*
 * Process commands
 * vmode is 1 if in view mode, 0 if in normal mode
 */
int sortBy;

int command(int vmode)
{
  int c; //creates a int c
  switch(c = getch()) { //gets an input and reads what the user is looking for
  case 'n': //n:  Move the cursor to the next line (if any).
    next_line();
    break;
  case 'N': //N:  Move the cursor to the next screen (if any).
    next_screen();
    break;
  case 'p': //p:  Move the cursor to the previous line (if any).
    previous_line();
    break;
  case 'P': //P:  Move the cursor to the previous screen (if any).
    previous_screen();
    break;
  case 'q': //q:  Quit the program.
    if(!vmode) return(1); //if we are in normal mode return success
    else feep(""); //if we are in view mode, do nothing
    break;
  case 'o': //o:  Open the directory at the current line and display an indented list of its contents.
    if(!vmode) open_directory(cursor_node);
    else feep(""); //if we are in view mode, do nothing
    break;
  case 'c': //c:  Close the directory at the current line, removing the indented list of its contents.
    if(!vmode) close_directory(cursor_node);
    else feep(""); //if we are in view mode, do nothing
    break;
  case 'v': //v:  Open the file at the current line and enter "view mode".
    if(!vmode) view_file(cursor_node); //view mode allows us to back into past lines
    else feep(""); //if we are in view mode, do nothing
    break;
  case 033:  /* ESC */ //allows for cases of arrow keys and del
    if(vmode) return(1); //if we are in vmode, return 1
    else feep(""); //return an error
    break;
  case ERR: //any error such as EOF/getch() return fails
    return -1;
  case '\f': //\f ("form feed", typed as CTRL-L):  Clear the screen and refresh the contents.
    refreshdisplay();
  default:
    feep(""); //do nothing basically
    break;
  }
  return(0); //return 0 so we continue running
}

/*
 * Move cursor to the next line of information
 */

static void next_line()
{
  if(cursor_node->next != NULL) {
    cursor_node = cursor_node->next; //FOUND A BUG
    if(cursor_line < screen_height-1) cursor_line++;
  } else {
    feep("");
  }
}

/*
 * Move cursor to the previous line of information
 */

static void previous_line()
{
  if(cursor_node->prev != NULL) {
    cursor_node = cursor_node->prev;
    if(cursor_line > 0) cursor_line--;
  } else {
    feep("");
  }
}

/*
 * Move cursor to the next screen of information
 */

static void next_screen()
{
  int i;

  for(i = 0; i < screen_height-1; i++) {
    if(cursor_node->next != NULL) {
      cursor_node = cursor_node->next;
      if(cursor_line < screen_height-1) cursor_line++;
    }
  }
}

/*
 * Move cursor to the previous screen of information
 */

static void previous_screen()
{
  int i;

  for(i = 0; i < screen_height-1; i++) {
    if(cursor_node->prev != NULL) {
      cursor_node = cursor_node->prev;
      if(cursor_line > 0) cursor_line--;
    }
  }
}

/*
 * Insert nodes for the contents of a directory
 */

static void open_directory(NODE *dir)
{
  NODE *node, *new;
  DIR *d;
  struct dirent *dp;
  char path[MAXPATHLEN+1];

//This is for sorting the items and appending, not sorting it all together
  NODE *firstNode = NULL; //first node to sort by
  int firstFlag = 0; //alert when first node is defined
  int n = 0; //number of nodes

  /* Make sure it's a directory */
  if(dir->info == NULL
     || (dir->info->stat.st_mode & S_IFMT) != S_IFDIR) {
    feep("Not a directory");
    return;
  }
  if((d = opendir(dir->info->path)) == NULL) {
    feep("Can't read directory");
    return;
  }
  node = dir;
  while((dp = readdir(d)) != NULL) {
    /* Don't display ".." */
    if(!strcmp(dp->d_name, "..")) continue;
    strcpy(path, dir->info->path);
    if(strcmp(path, "/")) strcat(path, "/");
    strcat(path, dp->d_name);
    if((new = get_info(path)) == NULL) continue;
    if(new->info->stat.st_dev == dir->info->stat.st_dev
       && new->info->stat.st_ino == dir->info->stat.st_ino) {
      free(new->info);
  #ifdef NO_MAXLINE //if data is an array we don't need to free it
      if(new->data){
        free(new->data);
      }
  #endif
      free(new);
      continue;       /* Don't display '.' */
    }
    if(!firstFlag){ //get the first node
      firstNode=new;
      firstFlag=1;
    }
    n++; //keep track of all nodes we insert
    node = insert_node(node, new);
    node->info->parent = dir;
    node->info->level = dir->info->level+1;
  }
  NODE *currentNode = firstNode; //keep firstnode just in case
  for(; n>0; n--){ //insertion sort
    if(currentNode==NULL){
      break; //should never occur
    }
    NODE *futureNode = currentNode->next; //keep track of the future
    NODE *pastNode = currentNode->prev; //keep track of the past
    if(pastNode==NULL){
      break; //should never occur
    }
    if(pastNode!=NULL && pastNode != firstNode->prev) { //check if pastNode exists
      while(compareNode(currentNode,pastNode)<0) { //find where we belong
        if(pastNode->prev!=NULL && pastNode != firstNode->prev){
          pastNode=pastNode->prev; //positive means past goes first
        } else{
          break;
        }
      }
      if(pastNode->next == firstNode){
        firstNode=currentNode;
      }
      reinsert_node(pastNode, currentNode); //go there
    }
    currentNode = futureNode; //move on
  }
  free(d);
}

/*
 * Delete nodes under a particular directory
 */

void close_directory(NODE *dir)
{
  while(dir->next != NULL
	&& dir->next->info != NULL
	&& dir->next->info->parent == dir) {
    if((dir->next->info->stat.st_mode & S_IFMT) == S_IFDIR) {
      close_directory(dir->next);
      delete_node(dir);
    } else {
      delete_node(dir);
    }
  }
}

static int compareNode(NODE *first, NODE* second){
  if(first==NULL || second==NULL){
    return 0;
  }
  if(sortBy==1){ //sort by name
    return (strcasecmp(first->info->path,second->info->path));
  } else if(sortBy==2){ //sort by size
    return first->info->stat.st_size-second->info->stat.st_size;
  } else if(sortBy==3){ //sort by date/time
    if(first->info->stat.st_mtim.tv_sec==second->info->stat.st_mtim.tv_sec){ //if sec equal check nano
      if(second->info->stat.st_mtim.tv_nsec - first->info->stat.st_mtim.tv_nsec > 0){
        return 1; //catch cases of round to 0 i guess
      } else if(second->info->stat.st_mtim.tv_nsec - first->info->stat.st_mtim.tv_nsec < 0){
        return -1; //catch cases of round to 0 i guess
      }
      return 0; //if they are equal to the millisecond
    } else{ //one of the seconds are different
      if(second->info->stat.st_mtim.tv_sec - first->info->stat.st_mtim.tv_sec > 0){
        return 1; //catch cases of round to 0 i guess
      } else if(second->info->stat.st_mtim.tv_sec - first->info->stat.st_mtim.tv_sec < 0){
        return -1; //catch cases of round to 0 i guess
      }
      return 0; //should never reach here
    }
  }
  return 0; //sort by none/default, nodes are fine where they are
}
/*
 * Reinsert a node into a new position
 */
NODE *reinsert_node(NODE *old, NODE *new){
  //"delete" the node at its old spotwithout freeing it since we must reinsert it
  if(new==old){ //idk is this a case, must be
    return new;
  }
  new->prev->next = new->next; //the node before the new node should jump over it
  if(new->next!=NULL) new->next->prev = new->prev; //if there is a next node, it should jump back over it
  //reinsert it
  return insert_node(old,new); //Insert the node that is no longer in the "linked list" back in after old
}

/*
 * Process keyboard input and dispatch commands
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
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
  case 033:  /* ESC */ //not sure but its escape1
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
      free(new);
      continue;       /* Don't display '.' */
    }
    node = insert_node(node, new);
    node->info->parent = dir;
    node->info->level = dir->info->level+1;
  }
  //insertion sort(MAKE CASE INSENSITIVE//strcasecmp)
    //1. start with the first node (currentNode), and how long it goes(n)
    //2. for(int i =0; i < n; i++){
    //  a. futureNode = currentNode->next //keep track of the future
    //  b. pastNode = currentNode->prev //keep track of the past
    //  b. while(currentNode->data < pastNode->data){ pastNode=pastNode->prev } //find where we belong
    //  c. insert_node(pastNode, currentNode) //go there
    //  c.5 delete node
    //  d. currentNode = futureNode //move on
    //3. } //lmao
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

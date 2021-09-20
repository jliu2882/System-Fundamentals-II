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
  FILE *f;
  char buf[MAXLINE+1];
  int save_cursor_line = cursor_line;
  NODE *save_cursor_node = cursor_node;
  NODE *first, *last, *new, *next;

  if(node->info == NULL
     || (((node->info->stat.st_mode & S_IFMT) != S_IFREG)
	 && ((node->info->stat.st_mode & S_IFMT) != S_IFLNK))) {
    feep("Not a regular file or link");
    return;
  }
  if ((f = fopen(node->info->path, "r")) == NULL) {
    feep("Can't open file");
    return;
  }
  first = last = NULL;
  while((new = malloc(sizeof(NODE))) != NULL) {
    if(fgets(buf, MAXLINE, f) == NULL) break;
    strncpy(new->data, buf, MAXLINE);
    if(first == NULL) first = last = new;
    else last = insert_node(last, new);
  }
  fclose(f);
  if(first != NULL) {
    cursor_node = first;
    cursor_line = 0;
    do { redisplay(); } while(!command(1));
  } else {
    feep("Empty file");
    return;
  }
  while(first->next != NULL) {
    next = first->next;  /* Save because delete_node frees its arg */
    delete_node(first);
  }
  free(first);
}

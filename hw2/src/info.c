
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

#include "browse.h"

static void cvt_info(FILE_INFO *info, char *buf);
static char *cvt_mode(mode_t mode);

/*
 * Construct an information node, given a pathname
 */

NODE *get_info(char *path)
{
  FILE_INFO *info;
  NODE *node;

  if((info = malloc(sizeof(FILE_INFO))) == NULL
     || (node = malloc(sizeof(NODE))) == NULL) {
    feep("Out of memory");
    return(NULL);
  }
  info->parent = NULL;
  node->info = info;
  node->info->level = 0;
  node->next = node->prev = NULL;
  strcpy(info->path, path);
  if(stat(path, &info->stat) < 0) {
    feep("Can't stat file");
    free(info);
    free(node);
    return(NULL);
  }
  cvt_info(info, node->data);
  return(node);
}

/*
 * Convert file information to a printable line
 */

static void cvt_info(FILE_INFO *info, char *buf)
{
  char *n;
  struct passwd *pw;

  if(strcmp(info->path, "/")
     && (n = rindex(info->path, '/')) != NULL)
      n++;
  else
      n = info->path;
  pw = getpwuid(info->stat.st_uid);
  sprintf(buf, "%.10s %3d %-8.8s %8qd %.12s %s",
	  cvt_mode(info->stat.st_mode),
	  info->stat.st_nlink,
	  pw != NULL ? pw->pw_name : "",
	  info->stat.st_size,
	  ctime(&info->stat.st_mtime)+4,
	  n);
}

/*
 * Convert mode bits to printable representation, a la "ls -l"
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
  new->prev = old;
  new->next = old->next;
  old->next = new;
  if(new->next != NULL) new->next->prev = new;
  return(new);
}

/*
 * Delete the node following a given node and free it
 */

void delete_node(NODE *node)
{
  NODE *next = node->next;

  if(next == NULL) return;
  node->next = next->next;
  if(node->next != NULL) node->next->prev = node;
  if(next->info != NULL) free(next->info);
  free(next);
}

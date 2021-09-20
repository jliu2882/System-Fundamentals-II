/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */

/*
 * Structure to hold information about a file or directory
 */
typedef struct file_info {
  int level;			/* Depth of nesting of subdirectories */
  char path[MAXPATHLEN+1];	/* Full pathname of file or directory */
  struct stat stat;		/* Stat buffer for file information */
  struct node *parent;		/* Pointer to parent directory */
} FILE_INFO;

/*
 * Structure containing information about a line to be
 * displayed on the screen.  This can be either a line of text
 * or a line of information about a file or directory.
 */
#ifndef NO_MAXLINE
#define MAXLINE 256	/* Maximum length of a line on the screen */
#endif

typedef struct node {
#ifdef NO_MAXLINE
  char *data;           /* Data line to be displayed on screen */
#else
  char data[MAXLINE+1];	/* Data line to be displayed on screen */
#endif
  FILE_INFO *info;	/* Null for text line, nonnull for file */
  struct node *next;
  struct node *prev;
} NODE;

extern NODE *cursor_node;
extern int cursor_line;
extern int screen_height, screen_width;

NODE *get_info(char *path);
NODE *insert_node(NODE *old, NODE *new);
void delete_node(NODE *node);

void initdisplay();
void redisplay();
void enddisplay();
void refreshdisplay();
void feep(char *msg);

int command(int vmode);
void view_file(NODE *node);
void close_directory(NODE *dir);

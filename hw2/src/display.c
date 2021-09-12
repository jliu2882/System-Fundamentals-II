
/*
 * Routines for manipulating the display
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <curses.h>

#include "browse.h"

NODE *cursor_node;
int cursor_line;
int screen_height;
int screen_width;

void initdisplay()
{
  initscr();
  noecho();
  cbreak();
  erase();
  screen_height = LINES-1;  /* Last line reserved for error message */
  screen_width = COLS;
  cursor_line = 0;
  move(0, 0);
  refresh();
}

void redisplay()
{
  int first_line, last_line, indent;
  NODE *first_node, *last_node;

  first_line = cursor_line;
  first_node = cursor_node;
  while(first_node->prev != NULL && first_line > 0) {
    first_node = first_node->prev;
    first_line--;
  }
  last_line = first_line;
  last_node = first_node;
  while(last_node->next != NULL && last_line < screen_height-1) {
    last_node = last_node->next;
    last_line++;
  }
  erase();
  while(first_line <= last_line) {
    move(first_line, 0);
    if(first_node == cursor_node) {
      standout();
      clrtoeol();
      move(first_line, 0);
      addch('>');
    } else {
      addch(' ');
    }
    if(first_node->info != NULL)
      indent = first_node->info->level;
    else
      indent = 0;
    while(indent--) addch(' ');
    addstr(first_node->data);
    if(first_node == cursor_node) standend();
    first_node = first_node->next;
    first_line++;
  }
  move(cursor_line, 0);
  refresh();
}

void enddisplay()
{
  erase();
  refresh();
  endwin();
}

/*
 * Refresh the display contents.
 */
void refreshdisplay() {
  clearok(stdscr, TRUE);
  refresh();
}

/*
 * Display an error message and beep
 */

void feep(char *msg)
{
  fputc('\a', stderr);
  if(*msg) {
    standout();
    mvaddstr(LINES-1, 0, msg);
    standend();
    refresh();
    sleep(1);
  }
}

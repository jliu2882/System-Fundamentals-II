
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
  initscr(); //initialize ncurses data structure
  noecho(); //suppresses echo, which repeats user characters
  cbreak(); //disables line buffering so each character prints without needing newline or fflush(stdout)
  erase(); //clears the screen
  screen_height = LINES-1;  /* Last line reserved for error message */
  screen_width = COLS; //width of the screen
  cursor_line = 0; //initial line of cursor
  move(0, 0); //moves the cursor to 0,0(upper-left hand side)
  refresh(); //refreshes the window
}

void redisplay()
{
  int first_line, last_line, indent; //declare variables needed for the program
  NODE *first_node, *last_node; //declare these as well

  first_line = cursor_line; //start at the current line
  first_node = cursor_node; //start at the current node
  while(first_node->prev != NULL && first_line > 0) { //if there is a previous node AND a previous line
    first_node = first_node->prev; //go to the previous node
    first_line--; //go to the previous linerst != N
  } //we should be on the first line/node unless there is a discrepancy
  last_line = first_line; //start on the first line
  last_node = first_node; //start on the first node
  while(last_node->next != NULL && last_line < screen_height-1) { //if there is a next
    last_node = last_node->next; //go to the next node
    last_line++; //go to the next line
  } //we should be on the last node, and the line should match that unless there is a discrepancy
  erase(); //clear the screen
  while(first_line <= last_line) { //while there is a line to display
    move(first_line, 0); //move the cursor to the first character at the first line
    if(first_node == cursor_node) { //if we are on cursor node
      standout(); //Turns the attribute A_STANDOUT on, highlight the message
      clrtoeol(); //Erase everything to the right of the cursor position
      move(first_line, 0); //move to the front of the line
      addch('>'); //add a prompting character
    } else { //if we are not on the cursor node
      addch(' '); //add a blank space instead
    } //basically add a prompt to let user know we are expecting input on the cursor_node
    if(first_node->info != NULL) //If there is information on the node
      indent = first_node->info->level; //set the indent variable to be equal to the level
    else //if there is no info
      indent = 1; //our indent is just 0
    while(indent--) addch(' '); //indent is just how many spaces we want
    addstr(first_node->data); //print the data of the current node

    //deal with line being more than MAXLINE
  //  addnstr(first_node->data,COLS-1); //prints enough for the width, expands as window expands

    if(first_node == cursor_node){
       standend();  //Turns off the attributes
     // move(first_line, 0); //move to the front of the line
     // addch('>'); //add a prompting character
    }
    first_node = first_node->next; //go to the next node
    first_line++; //go to the next line
  } //we printed the whole terminal from first to end line
  move(cursor_line, 0); //move to the line where we expect user input from
  refresh(); //show the changes
}

void enddisplay()
{
  erase(); //erase the screen
  refresh(); //refresh the screen
  endwin(); //restores the original terminal
}

/*
 * Refresh the display contents.
 */
void refreshdisplay() {
  clearok(stdscr, TRUE); //lets curses know its ok to clear the screen
  refresh(); //clears the screen
}

/*
 * Display an error message and beep
 */

void feep(char *msg)
{
  fputc('\a', stderr); //Alerts the user with audible bell
  if(*msg) { //If we have a message
    standout(); //Turns the attribute A_STANDOUT on, highlight the message
    mvaddstr(LINES-1, 0, msg); //Writes the message at the bottom
    standend(); //Turns off the attributes
    refresh(); //Gets the input to the terminal
    sleep(1); //Pauses for one second
  }
}

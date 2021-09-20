/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */

#include <stdio.h>
#include <ncurses.h>

int LINES = 24; // Read-only variable
int COLS = 80;  // Read-only variable
WINDOW *stdscr = (void *)0x1badd00d;

WINDOW *initscr(void) {
    fprintf(stdout, "initscr()\n");
    return NULL;
}

int endwin(void) {
    fprintf(stdout, "endwin()\n");
    return 0;
}

int noecho(void) {
    fprintf(stdout, "noecho()\n");
    return 0;
}

int cbreak(void) {
    fprintf(stdout, "cbreak()\n");
    return 0;
}

int clearok(WINDOW *win, bool b) {
    fprintf(stdout, "clearok(%p, %d)\n", win, b);
    return 0;
}

int werase(WINDOW *win) {
    fprintf(stdout, "erase(%p)\n", win);
    return 0;
}

int wrefresh(WINDOW *win) {
    fprintf(stdout, "refresh(%p)\n", win);
    return 0;
}

int wclrtoeol(WINDOW *win) {
    fprintf(stdout, "clrtoeol(%p)\n", win);
    return 0;
}

int wmove(WINDOW *win, int r, int c) {
    fprintf(stdout, "move(%p, %d, %d)\n", win, r, c);
    return 0;
}

int waddch(WINDOW *win, const chtype c) {
    fprintf(stdout, "addch(%p, %d)\n", win, c);
    return 0;
}

static const char *escape(const char *s) {
    // TO BE DONE
    return s;
}

int waddnstr(WINDOW *win, const char *s, int n) {
    fprintf(stdout, "waddnstr(%p, \"%s\", %d)\n", win, escape(s), n);
    return 0;
}

int wgetch(WINDOW *win) {
    fprintf(stdout, "getch(%p): ", win);
    fflush(stdout);
    int c = getchar();
    int n;
    while(c != EOF && (n = getchar()) != EOF && n != '\n')
	;
    return c;
}

int wattrset(WINDOW *win, int attrs) {
    fprintf(stdout, "wattrset(%p, 0x%x)\n", win, attrs);
    return 0;
}

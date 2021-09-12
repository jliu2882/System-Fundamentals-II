
/*
 * File system browser.
 * E. Stark
 * 11/3/93
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <curses.h>
#include <unistd.h>
#include <stdlib.h>

#include "browse.h"

int
main(int argc, char *argv[])
{
  int err;
  char *getcwd(), *base;

  if(argc == 2) {
    if(chdir(argv[1]) < 0) {
      fprintf(stderr, "Can't change directory to %s\n", argv[1]);
      exit(1);
    }
  } else if(argc != 1) {
    fprintf(stderr, "Usage: %s [dir]\n", argv[0]);
    exit(1);
  }
  base = getcwd(NULL, MAXPATHLEN+1);
  if((cursor_node = get_info(base)) == NULL) {
    fprintf(stderr, "Can't stat %s\n", base);
    exit(1);
  }
  initdisplay();
  cursor_line = 0;
  do redisplay(); while(!(err = command(0)));
  enddisplay();
  exit(err < 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

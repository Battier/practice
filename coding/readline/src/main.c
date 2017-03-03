/*A tiny application which demonstrates how to use the
   GNU Readline library.  This application interactively allows users
   to manipulate files and their modes. */

#include "cli.h"
/* Forward declarations. */
extern char *stripwhite ();
extern int execute_line (char *line);
extern void initialize_readline ( void );

/* When false, this means the user is done using this program. */
bool isQuit = false;

int main ()
{
    char *line, *s;
    int rc = 0;

    initialize_readline();	/* Bind our completer. */

    /* Loop reading and executing lines until the user quits. */
    while (!isQuit)
    {
        line = readline ("CLI-test> ");

        if (!line)
          break;

        /* Remove leading and trailing whitespace from the line.
           Then, if there is anything left, add it to the history list
           and execute it. */
        s = stripwhite (line);

        if (*s)
        {
            add_history (s);
            rc = execute_line (s);
        }

        free (line);
    }
    exit (rc);
}



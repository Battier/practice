#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include <readline/readline.h>
#include <readline/history.h>

/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
    char         *name;	/* User printable name of the function. */
    rl_icpfunc_t *func;	/* Function to call to do the job. */
    char         *doc;	/* Documentation for this function.  */
} COMMAND;


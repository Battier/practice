/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */
#include "cli.h"

/* Forward declarations. */
extern char *xmalloc ();
extern COMMAND commands[];
extern COMMAND *find_command (char *name);
static char *command_generator __P((const char *, int));
static char **auto_completion __P((const char *, int, int));
static char* dupstr (char *s);

/* Tell the GNU Readline library how to complete.  We want to try to
   complete on command names if this is the first word in the line, or
   on filenames if not. */
void initialize_readline ( void )
{
    rl_instream = stdin;
    rl_outstream = stderr;
    /* Allow conditional parsing of the ~/.inputrc file. */
    rl_readline_name = "cli-test";

    /* Tell the completer that we want a crack first. */
    rl_attempted_completion_function = auto_completion;
}

/* Attempt to complete on the contents of TEXT.  START and END
   bound the region of rl_line_buffer that contains the word to
   complete.  TEXT is the word to complete.  We can use the entire
   contents of rl_line_buffer in case we want to do some simple
   parsing.  Returnthe array of matches, or NULL if there aren't any. */
static char **auto_completion (const char *text, int start, int end)
{
    char **matches;
#define DUMP_USAGE 1
#if DUMP_USAGE
    static char *save_cmd = NULL;
#endif

    //make compiler happy.
    end = end;

    matches = (char **)NULL;

    /* If this word is at the start of the line, then it is a command
       to complete.  Otherwise it is the name of a file in the current
       directory. */
    if (start == 0)
      matches = rl_completion_matches (text, command_generator);
#if DUMP_USAGE
    else {
      printf ("\n%s Usage:\n\t%s\n", save_cmd, (find_command(save_cmd))->doc);
    }
    if (matches){
        if (save_cmd)
           free (save_cmd);
        save_cmd = dupstr (matches[0]);
    }
#endif

    return (matches);
}

static char* dupstr (char *s)
{
    char *r;

    r = xmalloc (strlen (s) + 1);
    strcpy (r, s);
    return (r);
}

/* Generator function for command completion.  STATE lets us
   know whether to start from scratch; without any state
   (i.e. STATE == 0), then we start at the top of the list. */
static char *command_generator (const char *text, int state)
{
    static int list_index, len;
    char *name;

    /* If this is a new word to complete, initialize now.  This
       includes saving the length of TEXT for efficiency, and
       initializing the index variable to 0. */
    if (!state)
    {
        list_index = 0;
        len = strlen (text);
    }

    /* Return the next name which partially matches from the
       command list. */
    while ((name = commands[list_index].name) != NULL)
    {
        list_index++;

        if (strncmp (name, text, len) == 0)
          return (dupstr(name));
    }

    /* If no names matched, then return NULL. */
    return ((char *)NULL);
}



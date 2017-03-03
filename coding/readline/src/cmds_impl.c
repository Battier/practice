/* **************************************************************** */
/*                                                                  */
/*                        Commands                           */
/*                                                                  */
/* **************************************************************** */
#include "cli.h"
/* Forward declarations. */
extern bool isQuit;
static void too_dangerous (char *caller);
static int valid_argument (char *caller, char *arg);
/* The names of functions that actually do the manipulation. */
static int com_list __P((char *));
static int com_view __P((char *));
static int com_history __P((char *));
static int com_rename __P((char *));
static int com_stat __P((char *));
static int com_pwd __P((char *));
static int com_delete __P((char *));
static int com_help __P((char *));
static int com_cd __P((char *));
static int com_quit __P((char *));

COMMAND commands[] = {
    { "cd",     com_cd,     "Change to directory DIR" },
    { "delete", com_delete, "Delete FILE" },
    { "help",   com_help,   "Display this text" },
    { "?",      com_help,   "Synonym for `help'" },
    { "list",   com_list,   "List files in DIR" },
    { "ls",     com_list,   "Synonym for `list'" },
    { "pwd",    com_pwd,    "Print the current working directory" },
    { "quit",   com_quit,   "Quit using cli-test" },
    { "rename", com_rename, "Rename FILE to NEWNAME" },
    { "stat",   com_stat,   "Print out statistics on FILE" },
    { "view",   com_view,   "View the contents of FILE" },
    { "history",com_history,"Dump the input commands history" },
    { (char *)NULL, (rl_icpfunc_t *)NULL, (char *)NULL }
};

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
COMMAND *find_command (char *name)
{
    register int i;

    for (i = 0; commands[i].name; i++)
      if (strcmp (name, commands[i].name) == 0)
        return (&commands[i]);

    return ((COMMAND *)NULL);
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char *stripwhite (char *string)
{
    register char *s, *t;

    for (s = string; whitespace (*s); s++)
      ;

    if (*s == 0)
      return (s);

    t = s + strlen (s) - 1;
    while (t > s && whitespace (*t))
      t--;
    *++t = '\0';

    return s;
}

/* Execute a command line. */
int execute_line (char *line)
{
    register int i;
    COMMAND *command;
    char *word;

    /* Isolate the command word. */
    i = 0;
    while (line[i] && whitespace (line[i]))
      i++;
    word = line + i;

    while (line[i] && !whitespace (line[i]))
      i++;

    if (line[i])
      line[i++] = '\0';

    command = find_command (word);

    if (!command)
    {
        fprintf (stderr, "%s: No such command for CLI-test.\n", word);
        return (-1);
    }

    /* Get argument to command, if any. */
    while (whitespace (line[i]))
      i++;

    word = line + i;

    /* Call the function. */
    return ((*(command->func)) (word));
}

/* String to pass to system ().  This is for the LIST, VIEW and RENAME
   commands. */
static char syscom[1024];

/* List the file(s) named in arg. */
static int com_list (char *arg)
{
    if (!arg)
      arg = "";

    sprintf (syscom, "ls -FClg %s", arg);
    return (system (syscom));
}

static int com_history (char *arg)
{
    HIST_ENTRY **the_list;
    int i;

    //make compiler happy.
    arg = arg;

    the_list = history_list ();
    if (the_list)
      for (i = 0; the_list[i]; i++)
        printf ("%d: %s\n", i + history_base, the_list[i]->line);
    return 0;

}

static int com_view (char *arg)
{
    if (!valid_argument ("view", arg))
      return 1;

    sprintf (syscom, "more %s", arg);
    return (system (syscom));
}

static int com_rename (char *arg)
{
    //make compiler happy.
    arg = arg;

    too_dangerous ("rename");
    return (1);
}

static int com_stat (char *arg)
{
    struct stat finfo;

    if (!valid_argument ("stat", arg))
      return (1);

    if (stat (arg, &finfo) == -1)
    {
        perror (arg);
        return (1);
    }

    printf ("Statistics for `%s':\n", arg);

    printf ("%s has %ld link%s, and is %ld byte%s in length.\n", arg,
                finfo.st_nlink,
                (finfo.st_nlink == 1) ? "" : "s",
                finfo.st_size,
                (finfo.st_size == 1) ? "" : "s");
    printf ("Inode Last Change at: %s", ctime (&finfo.st_ctime));
    printf ("      Last access at: %s", ctime (&finfo.st_atime));
    printf ("    Last modified at: %s", ctime (&finfo.st_mtime));
    return (0);
}

static int com_delete (char *arg)
{
    //make compiler happy.
    arg = arg;

    too_dangerous ("delete");
    return (1);
}

/* Print out help for ARG, or for all of the commands if ARG is
   not present. */
static int com_help (char *arg)
{
    register int i;
    int printed = 0;

    for (i = 0; commands[i].name; i++)
    {
        if (!*arg || (strcmp (arg, commands[i].name) == 0))
        {
            printf ("%s\t\t%s.\n", commands[i].name, commands[i].doc);
            printed++;
        }
    }

    if (!printed)
    {
        printf ("No commands match `%s'.  Possibilties are:\n", arg);

        for (i = 0; commands[i].name; i++)
        {
            /* Print in six columns. */
            if (printed == 6)
            {
                printed = 0;
                printf ("\n");
            }

            printf ("%s\t", commands[i].name);
            printed++;
        }

        if (printed)
          printf ("\n");
    }
    return (0);
}

/* Change to the directory ARG. */
static int com_cd (char *arg)
{
    int rc = 0;
    if (chdir((const char *)arg) == -1)
    {
        perror (arg);
        return 1;
    }

    rc = com_pwd ("");
    return (rc);
}

/* Print out the current working directory. */
static int com_pwd (char *arg)
{
    char dir[1024], *s;

    //make compiler happy.
    arg = arg;

    s = getcwd(dir, sizeof(dir) - 1);
    if (s == 0)
    {
        printf ("Error getting pwd: %s\n", dir);
        return 1;
    }

    printf ("Current directory is %s\n", dir);
    return 0;
}

/* The user wishes to quit using this program.  Just set isQuit
   non-zero. */
static int com_quit (char *arg)
{
    //make compiler happy.
    arg = arg;

    isQuit = true;
    return (0);
}

/* Function which tells you that you can't do this. */
static void too_dangerous (char *caller)
{
    fprintf (stderr, "%s: Too dangerous for me to distribute.\n", caller);
    fprintf (stderr, "Write it yourself.\n");
}

/* Return non-zero if ARG is a valid argument for CALLER,
   else print an error message and return zero. */
static int valid_argument (char *caller, char *arg)
{
    if (!arg || !*arg)
    {
        fprintf (stderr, "%s: Argument required.\n", caller);
        return (0);
    }

    return (1);
}

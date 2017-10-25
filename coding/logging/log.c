/********************************************************************
 *
 *      File:   log.c
 *      Name:   Lu Fanqi
 *
 *        Description:
 *        A syslog-based logging libary
 *
 *  # Please Notes:
 *  # Logging for examle system.
 *    local7.*	        	/var/log/lfqlog
 *    *.*;local7.none    	-/var/log/syslog
 *
 **********************************************************************/

#include "log.h"

const char* log_prefix_str[] = {
    "## Emergence : ",
    "## Alert : ",
    "## Critical : ",
    "## Error : ",
    "## Notice : ",
    "## Warn : ",
    "## Info : ",
    "## Debug : ",
};

int  g_slog_level = SLOG_DEFAULT_LEVEL;
int syslog_print_init(const char *name)
{
    int opt = LOG_CONS | LOG_PID | LOG_NDELAY;
    setlogmask (LOG_UPTO (g_slog_level));
    openlog (name, opt, SLOG_FACILITY);
    return 0;
}

void syslog_print (int priority, const char *fmt, ... )
{
    va_list args;
    va_start(args, fmt);    
    vsyslog(priority, fmt, args);  
    va_end(args);
}

void syslog_print_close(void)
{
    closelog ();
}

void syslog_set_level(int level)
{
    ASSERT(level >= LOG_EMERG && level <= LOG_DEBUG);
    g_slog_level = level;
    setlogmask(LOG_UPTO (g_slog_level));    
}

int syslog_get_level(void)
{
    return g_slog_level;
}

int  g_clog_level = CLOG_DEFAULT_LEVEL;
int console_print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);    
    vfprintf(stderr, fmt, args);
    va_end(args);
    return 0;
}

void console_set_level(int level)
{
    ASSERT(level >= LOG_EMERG && level <= LOG_DEBUG);
    g_clog_level = level;
}

int console_get_level(void)
{
    return g_clog_level;
}

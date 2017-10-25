/********************************************************************
 *
 *      File:   log.h
 *      Name:   Lu Fanqi
 *
 *        Description:
 *        A syslog-based logging libary
 *
 **********************************************************************/
#ifndef __LFQ_LOG_H
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SLOG_FACILITY  (LOG_LOCAL7)  
#define SLOG_FILE_PATH "/var/log/lfqlog.log"

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); return(-1); } while(0)

#ifndef SLOG_DEFAULT_LEVEL
#define SLOG_DEFAULT_LEVEL LOG_DEBUG
#endif

#define SLOG_PRINT(level, fmt, ...) do {	\
    syslog_print(level, "%s %s:%d: " fmt, log_prefix_str[level], __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while(0)

/*SLOG_EMERG: system is unusable */
#define SLOG_EMERG(fmt, ...)          SLOG_PRINT (LOG_EMERG,   fmt, ##__VA_ARGS__)

/*SLOG_ALERT: action must be taken immediately */
#define SLOG_ALERT(fmt, ...)          SLOG_PRINT (LOG_ALERT,   fmt, ##__VA_ARGS__)

/*SLOG_CRIT: critical conditions */
#define SLOG_CRIT(fmt, ...)           SLOG_PRINT (LOG_CRIT,    fmt, ##__VA_ARGS__)

/*SLOG_ERR: error conditions */
#define SLOG_ERR(fmt, ...)            SLOG_PRINT (LOG_ERR,     fmt, ##__VA_ARGS__)

/*SLOG_NOTICE: normal but significant condition */
#define SLOG_NOTICE(fmt, ...)         SLOG_PRINT (LOG_NOTICE,  fmt, ##__VA_ARGS__)

/*SLOG_WARNING: warning conditions  */
#define SLOG_WARNING(fmt, ...)        SLOG_PRINT (LOG_WARNING, fmt, ##__VA_ARGS__)

/*SLOG_INFO: informational */
#define SLOG_INFO(fmt, ...)           SLOG_PRINT (LOG_INFO,    fmt, ##__VA_ARGS__)

/*SLOG_DEBUG: debug-level messages */
#define SLOG_DEBUG(fmt, ...)          SLOG_PRINT (LOG_DEBUG,   fmt, ##__VA_ARGS__)

#ifndef CLOG_DEFAULT_LEVEL
#define CLOG_DEFAULT_LEVEL LOG_NOTICE
#endif

/*CLOG_PRINT: Print messages on console */
//First print to serial console    
//Second print to syslog  
#define CLOG_PRINT(level, fmt, ...) do {	\
    if (level <= g_clog_level) { \
        console_print("%s %s:%d: " fmt, log_prefix_str[level], __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
    SLOG_PRINT (level,   fmt, ##__VA_ARGS__); \
} while(0)

/*CLOG_EMERG: system is unusable */
#define CLOG_EMERG(fmt, ...)          CLOG_PRINT (LOG_EMERG,   fmt, ##__VA_ARGS__)

/*CLOG_ALERT: action must be taken immediately */
#define CLOG_ALERT(fmt, ...)          CLOG_PRINT (LOG_ALERT,   fmt, ##__VA_ARGS__)

/*CLOG_CRIT: critical conditions */
#define CLOG_CRIT(fmt, ...)           CLOG_PRINT (LOG_CRIT,    fmt, ##__VA_ARGS__)

/*CLOG_ERR: error conditions */
#define CLOG_ERR(fmt, ...)            CLOG_PRINT (LOG_ERR,     fmt, ##__VA_ARGS__)

/*CLOG_NOTICE: normal but significant condition */
#define CLOG_NOTICE(fmt, ...)         CLOG_PRINT (LOG_NOTICE,  fmt, ##__VA_ARGS__)

/*CLOG_WARNING: warning conditions  */
#define CLOG_WARNING(fmt, ...)        CLOG_PRINT (LOG_WARNING, fmt, ##__VA_ARGS__)

/*CLOG_INFO: informational */
#define CLOG_INFO(fmt, ...)           CLOG_PRINT (LOG_INFO,    fmt, ##__VA_ARGS__)

/*CLOG_DEBUG: debug-level messages */
#define CLOG_DEBUG(fmt, ...)          CLOG_PRINT (LOG_DEBUG,   fmt, ##__VA_ARGS__)

#define ASSERT(expr) do {\
    if (!(expr)) {\
        CLOG_ERR("%s", "Assert error");\
    }\
    assert(expr);\
} while(0)

extern const char* log_prefix_str[];
extern int  g_slog_level;
extern int  g_clog_level;
extern int  syslog_print_init(const char *name);
extern void syslog_print (int priority, const char *fmt, ... );
extern void syslog_print_close(void);
extern void syslog_set_level(int level);
extern int syslog_get_level(void);
extern int  console_print(const char *fmt, ...);
extern void console_set_level(int level);
extern int console_get_level(void);

#ifdef __cplusplus
}
#endif

#endif //__LFQ_LOG_H

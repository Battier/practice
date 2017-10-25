/********************************************************************
 *
 *      File:   log_test.c
 *      Name:   Lu Fanqi
 *
 *        Description:
 *        A simple example code for the logging libary
 *
 *  # Please Notes:
 *  # Logging for examle system.
 *    local7.*	        	/var/log/lfqlog
 *    *.*;local7.none    	-/var/log/syslog
 *
 **********************************************************************/
#include "log.h"

int main (int argc, char **argv)
{
    syslog_print_init("LFQ");
    SLOG_EMERG("%s(%d): %s", __FUNCTION__, __LINE__, "ABCDES");
    SLOG_ALERT("%s(%d): %s", __FUNCTION__, __LINE__, "ABCDES");
//    syslog_set_level(LOG_EMERG);
    SLOG_CRIT("%s(%d): %s", __FUNCTION__, __LINE__, "ABCDES");
    SLOG_ERR("%s(%d): %s", __FUNCTION__, __LINE__, "ABCDES");
    SLOG_NOTICE("%s(%d): %s", __FUNCTION__, __LINE__, "ABCDES");
    SLOG_WARNING("%s(%d): %s", __FUNCTION__, __LINE__, "ABCDES");
    SLOG_INFO("%s(%d): %s", __FUNCTION__, __LINE__, "ABCDES");
    SLOG_DEBUG("%s(%d): %s", __FUNCTION__, __LINE__, "ABCDES");
    CLOG_DEBUG("%s(%d): %s", __FUNCTION__, __LINE__, "ABCDES");
    syslog_print_close();
}

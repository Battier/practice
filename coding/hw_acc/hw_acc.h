/********************************************************************
 *
 *      File:   hw_acc.h
 *      Name:   Lu Fanqi
 *
 *        Description:
 *        app for hardware access implementation
 *
 **********************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#ifndef __ppc__
#include <sys/io.h>
#endif
#include <sys/mman.h>

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); return(-1); } while(0)

#define LENGTH_4_BYTES  (4)
typedef enum {
    MEM_SPACE,
    PORT_SPACE,
    DEV_MAX,
}dev_index;

typedef enum {
    DEVICE_INDEX,
    ACCESS_INDEX,
    OFFSET_INDEX,
    VALUE_INDEX,
    LENGTH_INDEX,
    OPTION_INDEX_MAX,
}option_index;

typedef struct __option_entry{
    char *str;
    option_index index;
}option_entry;

static option_entry key_mapping[] = 
{
    {"dev",    DEVICE_INDEX},
    {"device", DEVICE_INDEX},
    {"acc",    ACCESS_INDEX},
    {"access", ACCESS_INDEX},
    {"off",    OFFSET_INDEX},
    {"offset", OFFSET_INDEX},
    {"val",    VALUE_INDEX},
    {"value",  VALUE_INDEX},
    {"len",    LENGTH_INDEX},
    {"lenght", LENGTH_INDEX},
    {"?",      OPTION_INDEX_MAX},
    {"help",   OPTION_INDEX_MAX},
    {NULL,     OPTION_INDEX_MAX},
};

typedef struct __brd_util_param{
    dev_index    dev;
    uint64_t     offset;
    uint32_t     value;
    uint32_t     length;
    bool         is_read;
}brd_util_param;



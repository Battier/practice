/********************************************************************
 *
 *      File:   hw_acc.c
 *      Name:   Lu Fanqi
 *
 *        Description:
 *        app for hardware access implementation
 *
 *      Please note:
 *          make sure this CONFIG_STRICT_DEVMEM kernel option is disable.
 *        This's a security feature to prevent user space access to physical memory above 1MB(IIRC).
 *        But your address is quite likely to be non-RAM (PCI MMIO), it's also able to be map even
 *        with STRICT_DEVMEM.
 *        check CMD: grep 'CONFIG_STRICT_DEVMEM\=y' /boot/config-$(uname -r)
 *        
 ***********************************************************************/
#include "hw_acc.h"

static void display_usage (char *app_name)
{
    printf("%s Usage: [dev] [acc] [off] [val]\n", app_name);
    printf("\t<device/dev=mem/port> should be mem/port, \n\
                mem is memory, port is I/O port\n");
    printf("\t<access/acc=read/r/write/w>\n");
    printf("\t<offset/off=reg_offset/phy_addr>\n"); 
    printf("\t<value/val=value>\n");
    printf("\n");
}

static void _brd_util_param_init(brd_util_param *util_param)
{
    util_param->dev     = MEM_SPACE;
    util_param->offset  = 0;
    util_param->value   = 0;
    util_param->length  = LENGTH_4_BYTES;
    util_param->is_read = true;
}

static option_index _get_option_key( const char* str )
{
    int i = 0;
    while(NULL != key_mapping[i].str)
    {
        if(!strncasecmp(key_mapping[i].str, str, strlen(key_mapping[i].str)))
            return(key_mapping[i].index);
        i++;
    }
    return(key_mapping[i].index);
}

static char* _get_option_val(char * str)
{
    int i = 0;
    while('\0' != str[i] || ' ' != str[i])
    {
        if ('=' == str[i])
            return (str + i + 1);
        i++;
    }
    return NULL;
}

static int _parse_cmdline(int argc, char* argv[], brd_util_param *util_param)
{
    int  i    = 0;
    char *tmp = NULL;
    int index;

    _brd_util_param_init(util_param);

    if(argc <= 1)  FATAL;

    while(++i < argc) {
        index = _get_option_key( argv[i] );
        tmp   = _get_option_val( argv[i] );

        if(!tmp || ('\0' == tmp[0]) || (' ' == tmp[0])) FATAL;

        switch(index){
            case DEVICE_INDEX:
                if (strncasecmp("mem", tmp, 3) == 0){
                    util_param->dev = MEM_SPACE;
                }else if (strncasecmp("port", tmp, 3) == 0){
                    util_param->dev = PORT_SPACE;
                } else {
                     FATAL;
                }
            break;

            case ACCESS_INDEX:
                if ('w' == tmp[0] || 'W' == tmp[0]){
                    util_param->is_read = false;
                }else if ('r' == tmp[0] || 'R' == tmp[0]){
                    util_param->is_read = true;
                }else {
                     FATAL;
                }
            break;

            case OFFSET_INDEX:
                util_param->offset = (uint64_t)strtoull(tmp, NULL, 0);
            break;

            case VALUE_INDEX:
                util_param->value = (uint32_t)strtoul(tmp, NULL, 0);
            break;

            case LENGTH_INDEX:
                util_param->length = (uint32_t)strtoul(tmp, NULL, 0);

            case OPTION_INDEX_MAX:
                 FATAL;
        }
    }
    return 0;
}

static int _dump_mem(void *virt, uint32_t length)
{
    uint32_t *pData32 = (uint32_t *)virt;
    uint32_t i = 0;
    do {
        if (!i%16) printf("\n");
        printf("0x%x\t", *(pData32 + i));
    } while(length > (i+1)*4);
}

static int _memory_access(uint64_t offset, uint32_t *val, uint32_t length, bool is_read)
{
    int fd;
    off_t page_size = sysconf(_SC_PAGE_SIZE);
    void *map_base, *virt_addr; 

    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;

    /* Map page aligned*/
#ifdef PHY_ADDR_64BIT
    map_base = mmap64(0, (length/page_size + 1)*page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset & ~((__off64_t)page_size - 1));
#else
// map_base = mmap(0, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)offset & ~(page_size - 1));
    map_base = mmap(0, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)offset);
#endif
    if(map_base == NULL || errno) {
        close(fd);
        FATAL;
    }

    virt_addr = map_base + (offset & (page_size - 1));

    if(is_read){
        *val = *((uint32_t *) virt_addr);
        if (length > LENGTH_4_BYTES) _dump_mem(virt_addr, length);
    } else {
        *((uint32_t *) virt_addr) = *val;
    }

    if(munmap(map_base, page_size)) {
        close(fd);
        FATAL;
    }

    close(fd);
    return 0;
}

#ifndef __ppc__
static int _port_access(uint32_t offset, uint32_t *val, bool is_read)
{
    if (ioperm(offset, 1, true))  FATAL;

    if(is_read){
        *(char *)val = inb(offset);
    } else {
        outb(*(char *)val, offset);
    }

    if (ioperm(offset, 1, false))   FATAL;

    return 0;
}
#else
static int _port_access( const uint32_t offset, uint32_t *val, bool is_read)
{
    printf("PowerPC doesn't support it.\n");
    return 0;
}
#endif

int main( int argc, char* argv[] ) 
{
    brd_util_param params;

    if(_parse_cmdline(argc, argv, &params)) {
        display_usage(argv[0]);
        return 0;
    }

    switch (params.dev) {
        case MEM_SPACE:
            if(_memory_access(params.offset, &params.value, params.length, params.is_read))  FATAL;
        break;

        case PORT_SPACE:
            if(_port_access(params.offset, &params.value, params.is_read))  FATAL;
        break;

        default:
            printf("Don't support %d.\n", params.dev);
            break;
    }

    if (params.length <= LENGTH_4_BYTES) {
        printf("%-15s : 0x%02lx\n", params.is_read ? "Read Offset" :"Write Offset", params.offset);
        printf("%-15s : 0x%02x\n", "Register Value", params.value);
    }
    return 0;
}


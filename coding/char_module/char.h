#ifndef __CHAR_H_
#define __CHAR_H_
#ifdef __KERNEL__
#include <linux/ioctl.h>
#else
#include <sys/ioctl.h>
#endif

#define CHAR_VER          "0.1"
#define CHAR_NAME         "char_test"
#define CHAR_MAJOR        0 /*If you know in advance which major number you want to start with*/
#define CHAR_MINOR        1
#define MAX_LCFPGA            3

#undef PRINTF  /* undef it, just in case */
#ifdef __KERNEL__
#  define PRINTF(fmt, args...)  do {\
                                     if (char_debug)\
                                          printk( KERN_DEBUG CHAR_NAME" %s(%d):"fmt, __FUNCTION__, __LINE__,  ##args);\
                                }while(0)
#else
#  define PRINTF(fmt, args...)  do {\
                                     if (char_debug)\
                                          printf(CHAR_NAME" %s(%d):"fmt, __FUNCTION__, __LINE__,  ##args);\
                                }while(0) 
#endif

/************ Internal Typedefs ***********************************************/
typedef struct _char_pci
{
    unsigned int     domain;
    unsigned int     bus;
    unsigned int     slot;
    unsigned int     function;
} char_pci;

typedef struct _char_pci_dma
{
    char_pci       pci;
    size_t             size;
	void               *virt;
#ifdef __KERNEL__
	dma_addr_t         dma;
	struct pci_dev     *dev;
#else
	unsigned long long dma;
	void               *dev;
#endif
} char_pci_dma;

/********************************************************
 *
 * Error codes
 *
 ********************************************************/
#define CHAR_EINTR        2
#define CHAR_EPERM        3
#define CHAR_EINVAL       4
#define CHAR_ENOMEM       5
#define CHAR_EDELETED     6
#define CHAR_ETIMEOUT     7
#define CHAR_EBUSY        8
#define CHAR_ECONFLICT    9
#define CHAR_EEMPTY      10
#define CHAR_EFULL       11

/********************************************************
 *
 * IOCTL numbers
 *
 ********************************************************/
#define CHAR_IOC_MAGIC 'd'
#define CHAR_IOC_MSI_EN       _IOW(CHAR_IOC_MAGIC, 1, char_pci)
#define CHAR_IOC_MSI_DIS      _IOW(CHAR_IOC_MAGIC, 2, int)
#define CHAR_IOC_MSI_CNT      _IOW(CHAR_IOC_MAGIC, 3, int)
#define CHAR_IOC_DMA_ALLOC    _IOW(CHAR_IOC_MAGIC, 4, int)
#define CHAR_IOC_DMA_FREE     _IOW(CHAR_IOC_MAGIC, 5, int)

#endif //__CHAR_H_

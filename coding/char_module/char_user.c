#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "char.h"
#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); return(-1); } while(0)

int main()
{
    int fd;
    int rc;
    int irq;
    char_pci pci;

    if((fd = open("/dev/" CHAR_NAME, O_RDWR | O_SYNC)) == -1)  FATAL;

    pci.bus   = 0x4;
    pci.slot  = 0x0;
    pci.function = 0x0;
    rc = ioctl(fd, CHAR_IOC_MSI_EN, &pci);
    if (rc < 0) {
        printf ("CHAR_IOC_MSI_EN failed:%d\n", rc);
        exit(-1);
    }
    irq = rc;
    printf ("CHAR_IOC_MSI_EN IRQ:%d\n", rc);

    rc = ioctl(fd, CHAR_IOC_MSI_CNT, rc);
    if (rc < 0) {
        printf ("CHAR_IOC_MSI_CNT failed:%d\n", rc);
        exit(-1);
    }
    printf ("CHAR_IOC_MSI_CNT :%d\n", rc);

    pci.bus   = 0x3;
    pci.slot  = 0x0;
    pci.function = 0x0;
    rc = ioctl(fd, CHAR_IOC_MSI_EN, &pci);
    if (rc < 0) {
        printf ("CHAR_IOC_MSI_EN failed:%d\n", rc);
        exit(-1);
    }
    irq = rc;
    printf ("CHAR_IOC_MSI_EN IRQ:%d\n", rc);

    rc = ioctl(fd, CHAR_IOC_MSI_DIS, irq);
    if (rc < 0) {
        printf ("CHAR_IOC_MSI_CNT failed:%d\n", rc);
        exit(-1);
    }
    printf ("CHAR_IOC_MSI_DIS :%d\n", rc);



    close(fd);
    exit(0);
}

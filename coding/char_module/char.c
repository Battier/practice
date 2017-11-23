#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/proc_fs.h>
#include "char.h"
static int                  major = CHAR_MAJOR;
static int                  minor = CHAR_MINOR;
static struct cdev          char_cdev;
static struct class*        char_class;
static struct device*       char_device;

static struct file_operations char_fops;
static int char_pci_msi_disable(int irq);
static int char_debug = 1;

typedef struct _char_pci_msi
{
    char_pci     pci;
    atomic_t         counter;
	unsigned int     irq;
	struct pci_dev   *dev;
} char_pci_msi;

struct char_pci_info {
	char_pci_msi	msi[MAX_LCFPGA];
	char_pci_dma	dma[MAX_LCFPGA];
};
static struct char_pci_info device_info;

static irqreturn_t pci_msi_ISR(int irq, void *tl)
{
    int i = 0;
	/* disable the interrupt vector */
	disable_irq_nosync(irq);
	
    for (i = 0; i < MAX_LCFPGA; i++)
    {
        if (device_info.msi[i].irq == irq){
            atomic_inc(&(device_info.msi[i].counter));
            break;
        }
    }

    enable_irq(irq);
	return IRQ_HANDLED;
}


static int char_open(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static int char_release(struct inode *inode, struct file *file)
{
    int slot;
    for (slot = 0; slot < MAX_LCFPGA; slot++) {
        if(!device_info.msi[slot].irq)
            continue;
        PRINTF("Calling free_irq <%d>\n", device_info.msi[slot].irq);
        disable_irq_nosync(device_info.msi[slot].irq);
        free_irq(device_info.msi[slot].irq, &pci_msi_ISR);
        PRINTF("Done freeing IRQ.\n");
        pci_disable_msi(device_info.msi[slot].dev);
        device_info.msi[slot].irq = 0;
        atomic_set(&(device_info.msi[slot].counter), 0);        
    }
    return 0;
}

static int char_pci_msi_enable(char_pci *device)
{
    struct pci_dev *pci;
    int i = 0;
    int rc = 0;
    pci = pci_get_bus_and_slot(device->bus, 
                PCI_DEVFN(device->slot, device->function));
    if (pci == NULL) {
		printk(KERN_ERR "Don't find this PCI device(%d:%d.%d)\n", device->bus, 
                        device->slot, device->function);
		return -EFAULT;
    }

    printk(KERN_ERR "Found this PCI device(%x:%x.%x)\n", device->bus, 
                device->slot, device->function);

    if (pci_enable_device(pci)) {
        dev_info(&pci->dev, "pci_enable_device() FAILED");
        pci_disable_device(pci);
        return -EIO;
    }

    //Enable PCI bus mastering on FPGA
    pci_set_master(pci);

    rc = pci_alloc_irq_vectors(pci, 1, 1, PCI_IRQ_MSI);
    if (rc != 1)
    {
        printk(KERN_ERR "Failed to enable MSI, retrying to enable MSI-X \n");
        rc = pci_alloc_irq_vectors(pci, 1, 1, PCI_IRQ_MSIX);
    }

    if (rc != 1)
    {
        printk(KERN_ERR "FAILED TO ENABLE MSI! rc=%d !!! Bail out!\n", rc);
        return rc;
    } else {
        pci->irq = pci_irq_vector(pci, 0);
        printk(KERN_ERR "Enabled MSI for the FPGA PCI device. IRQ:%d\n", pci->irq);
    }

    PRINTF(" pci->irq:%d", pci->irq);
    if (request_irq(pci->irq, *pci_msi_ISR,
                    IRQF_SHARED,
                    "char",
                    &pci_msi_ISR) != 0) {
        panic("Can not assign IRQ %u to char\n", pci->irq);
        return -EFAULT;
    }

    for (i = 0; i < MAX_LCFPGA; i++ ) {
        if (device_info.msi[i].irq != 0 && device_info.msi[i].irq != pci->irq) {
            PRINTF("%d, %d", device_info.msi[i].irq, i);
            continue;
        }
        memcpy((void *)&(device_info.msi[i].pci), device, sizeof(char_pci));
        device_info.msi[i].irq = pci->irq;
        device_info.msi[i].dev = pci;
        atomic_set(&(device_info.msi[i].counter), 0);
        PRINTF( "Connected IRQ - %u slot %d\n", pci->irq, i);
        break;
    }

    if (i == MAX_LCFPGA){
        printk(KERN_ERR "Save IRQ info failed: %d\n", pci->irq);
        char_pci_msi_disable(pci->irq);
        return -EFAULT;
    }

    return pci->irq;
}

static int char_pci_msi_disable(int irq)
{
    struct pci_dev *pci = NULL;
    int i = 0;

    for (i = 0; i < MAX_LCFPGA; i++ ) {
        if (device_info.msi[i].irq == irq) {
            atomic_set(&(device_info.msi[i].counter), 0);
            device_info.msi[i].irq = 0;
            pci = device_info.msi[i].dev;
            PRINTF( "Dis-connected IRQ - %u slot %d\n", pci->irq, i);
        }
    }

    if (!pci) {
		printk(KERN_ERR "Don't find this PCI device.\n");
		return -EFAULT;
    }

    dev_info(&pci->dev, "Calling free_irq <%d>\n", pci->irq);
    free_irq(pci->irq, &pci_msi_ISR);
    dev_info(&pci->dev, "Done freeing IRQ.\n");
    pci_disable_msi(pci);
    return 0;
}

static int char_pci_msi_cnt(int irq)
{
    int i = 0;
    int rc = 0;

    for (i = 0; i < MAX_LCFPGA; i++ ) {
        if (device_info.msi[i].irq == irq) {
            rc = atomic_read(&(device_info.msi[i].counter));
            atomic_set (&(device_info.msi[i].counter), 0);
            PRINTF( "IRQ - %u slot %d counter %d\n", irq, i, rc);
        }
    }
    return rc;
}

static long char_ioctl(
        struct file *filp,
        unsigned int cmd,
        unsigned long arg
)
{
    char_pci     device;
    char_pci_dma dma;
    PRINTF(" cmd=0x%08X\n", cmd);
    /* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
    if (unlikely(_IOC_TYPE(cmd) != CHAR_IOC_MAGIC))
    {
        PRINTF("wrong ioctl magic key\n");
        return -ENOTTY;
    }
    PRINTF(" cmd=0x%08X\n", cmd);

    switch(cmd)
    {
        case CHAR_IOC_MSI_EN:
        {
            if (copy_from_user(&device, (char_pci *)arg, sizeof(device))) {
                printk(KERN_ERR "CHAR_IOC_MSI_EN: EFAULT\n");
                return -EFAULT;
            }
            return (char_pci_msi_enable(&device));
        }

        case CHAR_IOC_MSI_DIS:
            return (char_pci_msi_disable(arg));

        case CHAR_IOC_MSI_CNT:
            return (char_pci_msi_cnt(arg));

        case CHAR_IOC_DMA_ALLOC:
        {
            if (copy_from_user(&dma, (char_pci_dma *)arg, sizeof(dma))) {
                printk(KERN_ERR "CHAR_IOC_DMA_ALLOC: EFAULT\n");
                return -EFAULT;
            }            
            return 0;
        }

        case CHAR_IOC_DMA_FREE:
            return 0;

        default:
            return -ENOIOCTLCMD;
    }
    return 0;
}

static struct file_operations char_fops = {
	.owner          = THIS_MODULE,  
	.open           = char_open,
	.unlocked_ioctl = char_ioctl,
	.release        = char_release /* A.K.A close */
};

static char *char_devnode(struct device *dev, umode_t *mode)
{
	return kasprintf(GFP_KERNEL, "%s", dev->kobj.name);
}
#define CHAR_PROC
#include "char_proc.c"
static void char_cleanup(void)
{
#ifdef CHAR_PROC
    cleanup_procfs();
#endif
	char_release(NULL, NULL);
	device_destroy(char_class, MKDEV(major, minor));
	class_destroy(char_class);
	cdev_del(&char_cdev);
	unregister_chrdev_region(MKDEV(major, minor), 1);
}

static int char_init(void)
{
	int result = 0;
	dev_t dev;

	/* first thing register the device at OS */
	if (major != 0) {
		/* Register your major. */
		dev = MKDEV(major, minor);
		result = register_chrdev_region(dev, 1, CHAR_NAME);
	} else {
		/* get dynamic major */
		result = alloc_chrdev_region(&dev, minor, 1, CHAR_NAME);
		if (result == 0) {
			major = MAJOR(dev);
			minor = MINOR(dev);
			PRINTF("Got dynamic major for " CHAR_NAME ": %d\n", major);
		}
	}
	if (result < 0) {
		PRINTF(CHAR_NAME "_init: register_chrdev_region err= %d\n", result);
		return result;
	}

	cdev_init(&char_cdev, &char_fops);
	char_cdev.owner = THIS_MODULE;
	result = cdev_add(&char_cdev, dev, 1);
	if (result) {
		PRINTF(CHAR_NAME "_init: cdev_add err= %d\n", result);
error_region:
		unregister_chrdev_region(dev, 1);
		return result;
	}
	char_class = class_create(THIS_MODULE, CHAR_NAME);
	if (IS_ERR(char_class)) {
		printk(KERN_ERR "Error creating " CHAR_NAME " class.\n");
		cdev_del(&char_cdev);
		result = PTR_ERR(char_class);
		goto error_region;
	}

	char_class->devnode = char_devnode;
	char_device = device_create(char_class, NULL, dev, NULL, CHAR_NAME);

	memset(&device_info, 0, sizeof(device_info));
	PRINTF( "char major=%d minor=%d\n", major, minor);
#ifdef CHAR_PROC
    if (char_proc_init() < 0) {
        PRINTF( CHAR_NAME ": create procfs failed\n");
        device_destroy(char_class, MKDEV(major, minor));
        class_destroy(char_class);
        cdev_del(&char_cdev);
		unregister_chrdev_region(dev, 1);
        return -ENOMEM;
    }
#endif
	PRINTF( CHAR_NAME ": created, major=%d minor=%d\n", major, minor);

	return 0;
}

module_init(char_init);
module_exit(char_cleanup);

module_param(major, int, S_IRUGO);
module_param(minor, int, S_IRUGO);

MODULE_DESCRIPTION(CHAR_NAME);
MODULE_VERSION(CHAR_VER);
MODULE_AUTHOR("Fanqi Lu");
MODULE_LICENSE("GPL");

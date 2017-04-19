#ifndef __NL_H__
#define __NL_H__

#define NETLINK_USER 31

#undef PRINTF /* undef it, just in case */
#ifdef NL_DEBUG
#  ifdef __KERNEL__
#    define PRINTF(fmt, args...) printk( KERN_DEBUG "netlink: " fmt, ## args)
#  else
#    define PRINTF(fmt, args...) printf(fmt, ## args)
#  endif
#else
#  define PRINTF(fmt, args...) /* not debugging: nothing */
#endif

#endif //__NL_H__

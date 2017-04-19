#include <linux/version.h>
#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include "nl.h"

struct sock *nl_sk = NULL;

static void nl_recv_msg(struct sk_buff *skb) {

    struct nlmsghdr *nl_hdr;
    int             pid;
    struct sk_buff  *skb_out;
    int             msg_size;
    char            *msg = "Msg from kernel space!";
    int             res;

    PRINTF("Entering: %s\n", __FUNCTION__);

    msg_size = strlen(msg);

    nl_hdr = (struct nlmsghdr*)skb->data;

    PRINTF("Netlink received msg payload:%s\n", (char*)nlmsg_data(nl_hdr));

    /*pid of sending process */
    pid = nl_hdr->nlmsg_pid;

    skb_out = nlmsg_new(msg_size, 0);
    if(!skb_out)
    {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    } 

    nl_hdr = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);

    /* not in mcast group */
    NETLINK_CB(skb_out).dst_group = 0; 

    strncpy(nlmsg_data(nl_hdr), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if(res < 0)
      PRINTF("Error while sending bak to user\n");
}

static int __init nl_init(void) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)    
    struct netlink_kernel_cfg cfg = {
        .input = nl_recv_msg,
    };
#endif

    PRINTF("Entering: %s\n", __FUNCTION__);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)    
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
#else
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, nl_recv_msg, NULL, THIS_MODULE);
#endif
    if(!nl_sk)
    {
        printk(KERN_ALERT "Error creating socket.\n");
        return -1;
    }

    return 0;
}

static void __exit nl_exit(void) {
    PRINTF("exiting hello module\n");
    netlink_kernel_release(nl_sk);
}

module_init(nl_init); 
module_exit(nl_exit);
MODULE_AUTHOR("Fanqi Lu");
MODULE_LICENSE("GPL");


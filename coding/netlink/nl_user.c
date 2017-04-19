#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "nl.h"

/* maximum payload size*/
#define MAX_PAYLOAD 1024 

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); return(-1); } while(0)

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nl_hdr = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

int main()
{
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if(sock_fd < 0)
        FATAL;

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid    = getpid();

    if(bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)))
        FATAL;

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    /* For Linux Kernel */
    dest_addr.nl_pid    = 0; 
    /* unicast */
    dest_addr.nl_groups = 0; 

    nl_hdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if(!nl_hdr)
        FATAL;

    memset(nl_hdr, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nl_hdr->nlmsg_len   = NLMSG_SPACE(MAX_PAYLOAD);
    nl_hdr->nlmsg_pid   = getpid();
    nl_hdr->nlmsg_flags = 0;

    strcpy(NLMSG_DATA(nl_hdr), "Msg from user space!");

    iov.iov_base    = (void *)nl_hdr;
    iov.iov_len     = nl_hdr->nlmsg_len;
    msg.msg_name    = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov     = &iov;
    msg.msg_iovlen  = 1;

    PRINTF("Sending message to kernel\n");
    sendmsg(sock_fd, &msg, 0);
    PRINTF("Waiting for message from kernel\n");

    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    PRINTF("Received message payload: %s\n", (char *)NLMSG_DATA(nl_hdr));

    if(close(sock_fd)) {
        free(nl_hdr);
        FATAL;
    }
    free(nl_hdr);
}


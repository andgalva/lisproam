/*
 * andrea.c
 *
 *  Created on: Oct 8, 2013
 *      Author: andrea
 */


#include "andrea.h"

#include "lispd_tun.h"

#include <bits/sockaddr.h>
#include <asm/types.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <string.h>

#define NLMSG_BOTTOM(nlmsg) ((struct rtattr *)(((void *)(nlmsg)) + NLMSG_ALIGN((nlmsg)-> nlmsg_len)))


static int addAttr(struct nlmsghdr *nl_req, int attrlabel, const void *data, int datalen)
{
    struct rtattr *attr=NLMSG_BOTTOM(nl_req);
    unsigned int attrlen=RTA_LENGTH(datalen); /* sizeof(struct rtattr) + datalen + align */
    if(NULL==nl_req || (datalen > 0 && NULL==data))
    {
        printf("NULL arg detected!");
        return -1;
    }
    attr-> rta_type=attrlabel;
    attr-> rta_len=attrlen;
    memcpy(RTA_DATA(attr),data,datalen);

    nl_req-> nlmsg_len=NLMSG_ALIGN(nl_req-> nlmsg_len)+RTA_ALIGN(attrlen);
    return 0;
}
static struct rtattr * addNestedAttr(struct nlmsghdr *nl_req, int attrlabel)
{
    struct rtattr *nested = NLMSG_BOTTOM(nl_req);

    if(!addAttr(nl_req, attrlabel, NULL, 0))
        return nested;
    return NULL;
}
static void endNestedAttr(struct nlmsghdr *nl_req, struct rtattr *nested)
{
    nested-> rta_len = (void *)NLMSG_BOTTOM(nl_req) - (void *)nested;
}

void debugprint_msg(struct nlmsghdr *netlinkreq) {
    if(netlinkreq-> nlmsg_flags & NLM_F_ACK)
    {
        printf("Msg contains f_ack!");
    }
    if(!NLMSG_OK(netlinkreq,netlinkreq-> nlmsg_len))
    {
        printf("Looks like we're sending invalid nlmsg!! NLMSG_OK() == false at send!");
    }
    else
    {
        printf
        (
            "sending NLMSG: len %u, type %hu, flags %hu, seq %u pid %u\n",
            netlinkreq-> nlmsg_len,
            netlinkreq-> nlmsg_type,
            netlinkreq-> nlmsg_flags,
            netlinkreq-> nlmsg_seq,
            netlinkreq-> nlmsg_pid
        );
        switch(netlinkreq-> nlmsg_type)
        {
            case RTM_NEWROUTE:
            case RTM_DELROUTE:
            case RTM_GETROUTE:
            {
                printf
                (
                    "Req is route req (new %u, del %u, get %u)",
                    RTM_NEWROUTE,
                    RTM_DELROUTE,
                    RTM_GETROUTE
                );
                printf
                (
                    "family %u, dstlen %u, srclen %u, tos %u, table %u, proto %u, scope %u, type %u, flags %u",
                    (unsigned int)((struct rtmsg *) NLMSG_DATA(netlinkreq) )-> rtm_family,
                    (unsigned int)((struct rtmsg *) NLMSG_DATA(netlinkreq) )-> rtm_dst_len,
                    (unsigned int)((struct rtmsg *) NLMSG_DATA(netlinkreq) )-> rtm_src_len,
                    (unsigned int)((struct rtmsg *) NLMSG_DATA(netlinkreq) )-> rtm_tos,
                    (unsigned int)((struct rtmsg *) NLMSG_DATA(netlinkreq) )-> rtm_table,
                    (unsigned int)((struct rtmsg *) NLMSG_DATA(netlinkreq) )-> rtm_protocol,
                    (unsigned int)((struct rtmsg *) NLMSG_DATA(netlinkreq) )-> rtm_scope,
                    (unsigned int)((struct rtmsg *) NLMSG_DATA(netlinkreq) )-> rtm_type,
                    (unsigned int)((struct rtmsg *) NLMSG_DATA(netlinkreq) )-> rtm_flags
                );
                {
                    int len=netlinkreq-> nlmsg_len;
                    struct rtattr *at=(struct rtattr *)((char *)NLMSG_DATA(netlinkreq)+sizeof(struct rtmsg));
                    while(NULL!=at && RTA_OK(at,len))
                    {
                        char tmp[100];
                        switch(at-> rta_type)
                        {
                            case RTA_DST:
                                printf
                                (
                                    "dst is set to %s",
                                    inet_ntop
                                    (
                                        (at-> rta_len > 8)?AF_INET6:AF_INET,
                                        RTA_DATA(at),
                                        tmp,
                                        100
                                    )
                                );
                                break;
                            case RTA_SRC:
                                 printf
                                (
                                    "src is set to %s",
                                    inet_ntop
                                    (
                                        (at-> rta_len > 8)?AF_INET6:AF_INET,
                                        RTA_DATA(at),
                                        tmp,
                                        100
                                    )
                                );
                                break;
                            case RTA_GATEWAY:
                                printf
                                (
                                    "gw is set to %s",
                                    inet_ntop
                                    (
                                        (at-> rta_len > 8)?AF_INET6:AF_INET,
                                        RTA_DATA(at),
                                        tmp,
                                        100
                                    )
                                );
                                break;
                            case RTA_OIF:
                                printf
                                (
                                    "OIF is set to %u",
                                    *(unsigned int *)RTA_DATA(at)
                                );
                                break;
                            case RTA_PRIORITY:
                                printf
                                (
                                    "Priority is set to %u",
                                    *(unsigned int *)RTA_DATA(at)
                                );
                                break;
                            default:
                                printf("rta_type %u, len %u",at-> rta_type,at-> rta_len);
                                break;
                        }
                        at=RTA_NEXT(at,len);
                    }
                }
                break;
            }
            case RTM_NEWADDR:
            case RTM_GETADDR:
            case RTM_DELADDR:
                printf
                (
                    "Req is ADDR req (new %u, del %u, get %u)",
                    RTM_NEWADDR,
                    RTM_DELADDR,
                    RTM_GETADDR
                );
                printf
                (
                    "ifa_family %u, ifa_prefixlen %u, ifa_flags %u, ifa_scope %u, ifa_index %d",
                    (unsigned int)((struct ifaddrmsg *) NLMSG_DATA(netlinkreq) )-> ifa_family,
                    (unsigned int)((struct ifaddrmsg *) NLMSG_DATA(netlinkreq) )-> ifa_prefixlen,
                    (unsigned int)((struct ifaddrmsg *) NLMSG_DATA(netlinkreq) )-> ifa_flags,
                    (unsigned int)((struct ifaddrmsg *) NLMSG_DATA(netlinkreq) )-> ifa_scope,
                    (int)((struct ifaddrmsg *) NLMSG_DATA(netlinkreq) )-> ifa_index
                );
                {
                    int len=netlinkreq-> nlmsg_len;
                    struct rtattr *at=(struct rtattr *)((char *)NLMSG_DATA(netlinkreq)+sizeof(struct ifaddrmsg));
                    while(NULL!=at && RTA_OK(at,len))
                    {
                        char tmp[100];
                        switch(at-> rta_type)
                        {
                            case IFA_ADDRESS:
                                printf
                                (
                                    "IFA_ADDRESS is set to %s",
                                    inet_ntop
                                    (
                                        (at-> rta_len > 8)?AF_INET6:AF_INET,
                                        RTA_DATA(at),
                                        tmp,
                                        100
                                    )
                                );
                                break;
                            case IFA_LOCAL:
                                printf
                                (
                                    "IFA_LOCAL is set to %s",
                                    inet_ntop
                                    (
                                        (at-> rta_len > 8)?AF_INET6:AF_INET,
                                        RTA_DATA(at),
                                        tmp,
                                        100
                                    )
                                );
                                break;
                            case IFA_BROADCAST:
                                printf
                                (
                                    "IFA_BROADCAST is set to %s",
                                    inet_ntop
                                    (
                                        (at-> rta_len > 8)?AF_INET6:AF_INET,
                                        RTA_DATA(at),
                                        tmp,
                                        100
                                    )
                                );
                                break;
                            case IFA_LABEL:
                                printf
                                (
                                    "IFA_LABEL is set to '%s'",
                                    (char *)RTA_DATA(at)
                                );
                                break;
                            case IFA_ANYCAST:
                                printf
                                (
                                    "IFA_ANYCAST is set to %s",
                                    inet_ntop
                                    (
                                        (at-> rta_len > 8)?AF_INET6:AF_INET,
                                        RTA_DATA(at),
                                        tmp,
                                        100
                                    )
                                );
                                break;
                            default:
                                printf("rta_type %u, len %u",at-> rta_type,at-> rta_len);
                                break;
                        }
                        at=RTA_NEXT(at,len);
                    }
                }
                break;
        	}
    }
}


void create_vlan()
{
	struct nlmsghdr nl_msg;
	struct ifinfomsg if_msg;
	char buf[8192];

	// variables used for
	// socket communications
	int sd;
	struct sockaddr_nl local_addr;
	struct sockaddr_nl remote_addr;
	struct msghdr resp_msg;
	struct iovec iov;
	int rtn;


	// RTNETLINK message pointers & lengths
	// used when processing messages
	struct nlmsghdr *nlp;
	int nll;
	struct rtmsg *rtp;
	int if_msg_length;
	struct rtattr *rtap;


	/* ******* */

	// open socket
	sd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	// setup local address & bind using
	// this address
	bzero(&local_addr, sizeof(local_addr));
	local_addr.nl_family = AF_NETLINK;
	local_addr.nl_pid = getpid();
	bind(sd, (struct sockaddr*) &local_addr, sizeof(local_addr));


	// sub functions to create RTNETLINK message,
	// send over socket, reveive reply & process
	// message

	// initalize RTNETLINK request buffer
	bzero(&nl_msg, sizeof(nl_msg));
	bzero(&if_msg, sizeof(if_msg));

	// compute the initial length of the
	// service request
	if_msg_length = sizeof(struct ifinfomsg);

	// setup the NETLINK header
	nl_msg.nlmsg_len = NLMSG_LENGTH(if_msg_length);
	/*
	 * NLM_F_CREATE:	Create link if it does not exist
	 * NLM_F_EXCL:		Return EEXIST if link already exists
	 */
	nl_msg.nlmsg_flags = NLM_F_REQUEST | NLM_F_EXCL | NLM_F_CREATE;

	nl_msg.nlmsg_type = RTM_NEWLINK;
	nl_msg.nlmsg_seq = NL_SEQNUM++;
	nl_msg.nlmsg_pid = getpid();

	// setup the service header
	if_msg.ifi_family = 0;
	if_msg.ifi_flags = 0;
	if_msg.ifi_index = 0;
	if_msg.ifi_change = 0xffffffff;

	// create the remote address
	// to communicate
	bzero(&remote_addr, sizeof(remote_addr));
	remote_addr.nl_family = AF_NETLINK;

	// initialize & create the struct msghdr supplied
	// to the sendmsg() function
	bzero(&resp_msg, sizeof(resp_msg));
	resp_msg.msg_name = (void *) &remote_addr;
	resp_msg.msg_namelen = sizeof(remote_addr);

	// place the pointer & size of the RTNETLINK
	// message in the struct msghdr
	iov.iov_base = (void *) &nl_msg;
	iov.iov_len = nl_msg.nlmsg_len;
	resp_msg.msg_iov = &iov;
	resp_msg.msg_iovlen = 1;

	debugprint_msg(&nl_msg);
	// send the RTNETLINK message to kernel
	rtn = sendmsg(sd, &resp_msg, 0);

	retry:
	rtn = recvmsg(sd, &resp_msg, 0);
	if(rtn <0)
	{
		if (errno==EINTR)
			goto retry;
		else
			printf("Error when receiving from netlink sock!");
	}
	else
		printf("Received answer: %d\n", rtn);


	// initialize RTM_SETLINK request buffer
	bzero(&nl_msg, sizeof(nl_msg));
	bzero(&if_msg, sizeof(if_msg));

	// compute the initial length of the
	// service request
	if_msg_length = sizeof(struct ifinfomsg);

	// setup the NETLINK header
	nl_msg.nlmsg_len = NLMSG_LENGTH(if_msg_length);
	nl_msg.nlmsg_flags = NLM_F_REQUEST;
	nl_msg.nlmsg_type = RTM_SETLINK;
	nl_msg.nlmsg_seq = NL_SEQNUM++;
	nl_msg.nlmsg_pid = getpid();

	if_msg.ifi_family = AF_INET;
	if_msg.ifi_flags = IFF_UP;
	if_msg.ifi_index = ID_INTERFACE++;

	/* ATTRIBUTES */

    struct rtattr *attr1, *attr2;
	char ifname[10]={0};
	int cnt = 1;
	int vlanid = 1;

	if_indextoname(cnt,ifname);
	while (strcmp(ifname,"wlan0") != 0) {
		cnt++;
		if_indextoname(cnt,ifname);
	}
	//found eth0.1 (br-lisplan)
	printf("ETH0.1 index: %d\n",cnt);

	sprintf(ifname,"%s.%d", ifname, ID_INTERFACE++);

    if(addAttr(&nl_msg,IFLA_LINK,&cnt,sizeof(int)))
    {
        printf("IFLA_LINK %d adding as rtattr to req failed!",cnt);
        rtn=-1;
    }
    else if(addAttr(&nl_msg,IFLA_IFNAME,ifname,strlen(ifname)))
    {
        printf("IFLA_IFNAME %s adding as rtattr to req failed!",ifname);
        rtn=-1;
    }
    else if(NULL==(attr1=addNestedAttr(&nl_msg,IFLA_LINKINFO)))
    {
        printf("addNestedAttr IFLA_LINKINFO FAILED!");
        rtn=-1;
    }
    else if(addAttr(&nl_msg,IFLA_INFO_KIND,"vlan", strlen("vlan")))
    {
        printf("IFLA_INFO_KIND \"vlan\" adding FAILED!");
        rtn=-1;
    }
    else if(NULL==(attr2=addNestedAttr(&nl_msg,IFLA_INFO_DATA)))
    {
        printf("addNestedAttr IFLA_INFO_DATA FAILED!");
        rtn=-1;
    }
    else if(addAttr(&nl_msg,IFLA_VLAN_ID,&vlanid,sizeof(unsigned short)))
    {
        printf("IFLA_VLAN_ID %hu adding as rtattr to req failed!",vlanid);
        rtn=-1;
    }
    else
    {
        endNestedAttr(&nl_msg,attr2);
        endNestedAttr(&nl_msg,attr1);
        printf("VLAN ID %hu, orig ifindex %d and new ifname %s added as attrs",vlanid,cnt,ifname);
    }

	debugprint_msg(&nl_msg);

	// initialize & create the struct msghdr supplied
	// to the sendmsg() function
	bzero(&resp_msg, sizeof(resp_msg));
	resp_msg.msg_name = (void *) &remote_addr;
	resp_msg.msg_namelen = sizeof(remote_addr);

	// place the pointer & size of the RTNETLINK
	// message in the struct msghdr
	iov.iov_base = (void *) &nl_msg;
	iov.iov_len = nl_msg.nlmsg_len;
	resp_msg.msg_iov = &iov;
	resp_msg.msg_iovlen = 1;

	// send the RTNETLINK message to kernel
	rtn = sendmsg(sd, &resp_msg, 0);

	fuck:
	rtn = recvmsg(sd, &resp_msg, 0);
	if(rtn <0)
	{
		if (errno==EINTR)
			goto fuck;
		else
			printf("Error when receiving from netlink sock!");
	}
	else
		printf("Received answer: %d\n", rtn);

	// close socket
	close(sd);
}

void OLD_create_vlan()
{
	// buffer to hold the RTNETLINK request
	struct {
	struct nlmsghdr nl;
	struct ifinfomsg rt;
	char buf[8192];
	} req;

	// variables used for
	// socket communications
	int fd;
	struct sockaddr_nl la;
	struct sockaddr_nl pa;
	struct msghdr msg;
	struct iovec iov;
	int rtn;

	// buffer to hold the RTNETLINK reply(ies)
	char buf[8192];

	// RTNETLINK message pointers & lengths
	// used when processing messages
	struct nlmsghdr *nlp;
	int nll;
	struct rtmsg *rtp;
	int ifil;
	struct rtattr *rtap;

	/* ******* */


	// open socket
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	// setup local address & bind using
	// this address
	bzero(&la, sizeof(la));
	la.nl_family = AF_NETLINK;
	la.nl_pid = getpid();
	la.nl_groups = 0;
	bind(fd, (struct sockaddr*) &la, sizeof(la));


	// sub functions to create RTNETLINK message,
	// send over socket, reveive reply & process
	// message

	// initalize RTNETLINK request buffer
	bzero(&req, sizeof(req));

	// compute the initial length of the
	// service request
	ifil = sizeof(struct ifinfomsg);

	// add first attrib: set Interface name
	// RTNETLINK buffer size changes...
	rtap = (struct rtattr *) req.buf;
	rtap->rta_type = IFLA_IFNAME;
	rtap->rta_len = sizeof(struct rtattr) + 4;
	sprintf( ((char *)rtap) + sizeof(struct rtattr), "lisproam%d", ID_INTERFACE);
		//inet_pton(AF_INET, new_ifname, ((char *)rtap) + sizeof(struct rtattr));
	ifil += rtap->rta_len;

	printf("\t\tCreating interface: %s\n", ((char *)rtap) + sizeof(struct rtattr));

	/* add second attrib:
	// set ifc index and increment the size
	rtap = (struct rtattr *) (((char *)rtap)
	+ rtap->rta_len);
	rtap->rta_type = IFA_LOCAL;
	rtap->rta_len = sizeof(struct rtattr) + 4;
	inet_pton(AF_INET, dsts,
	((char *)rtap) + sizeof(struct rtattr));
	ifil += rtap->rta_len;
	 */

	// setup the NETLINK header
	req.nl.nlmsg_len = NLMSG_LENGTH(ifil);
	/*
	 * NLM_F_CREATE:	Create link if it does not exist
	 * NLM_F_EXCL:		Return EEXIST if link already exists
	 */
	req.nl.nlmsg_flags = NLM_F_EXCL | NLM_F_CREATE;

	req.nl.nlmsg_type = RTM_NEWLINK;

	// setup the service header (struct rtmsg)
	req.rt.ifi_family = AF_INET;
	//req.rt.ifa_prefixlen = 32; /*hardcoded*/
	req.rt.ifi_flags = IFA_F_PERMANENT;
	//req.rt.ifi_type =  ;
	/*TODO: think how can you find out the ifa_index */
	req.rt.ifi_index = ID_INTERFACE;
	ID_INTERFACE++;

	// create the remote address
	// to communicate
	bzero(&pa, sizeof(pa));
	pa.nl_family = AF_NETLINK;

	// initialize & create the struct msghdr supplied
	// to the sendmsg() function
	bzero(&msg, sizeof(msg));
	msg.msg_name = (void *) &pa;
	msg.msg_namelen = sizeof(pa);

	// place the pointer & size of the RTNETLINK
	// message in the struct msghdr
	iov.iov_base = (void *) &req.nl;
	iov.iov_len = req.nl.nlmsg_len;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	// send the RTNETLINK message to kernel
	rtn = sendmsg(fd, &msg, 0);

	// close socket
	close(fd);
}

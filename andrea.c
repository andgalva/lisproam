/*
 * andrea.c
 *
 *  Created on: Oct 8, 2013
 *      Author: andrea
 */


#include <bits/sockaddr.h>
#include <asm/types.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <string.h>

// buffer to hold the RTNETLINK request
struct {
struct nlmsghdr nl;
struct ifaddrmsg rt;
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
int ifal;
struct rtattr *rtap;


void create_vlan()
{

	// open socket
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	// setup local address & bind using
	// this address
	bzero(&la, sizeof(la));
	la.nl_family = AF_NETLINK;
	la.nl_pid = getpid();
	bind(fd, (struct sockaddr*) &la, sizeof(la));


	// sub functions to create RTNETLINK message,
	// send over socket, reveive reply & process
	// message

	/*TODO*/
	char dsts[24] = "10.10.10.3";

	// initalize RTNETLINK request buffer
	bzero(&req, sizeof(req));

	// compute the initial length of the
	// service request
	ifal = sizeof(struct ifaddrmsg);
	// add first attrib:
	// set IP addr
	// RTNETLINK buffer size
	rtap = (struct rtattr *) req.buf;
	rtap->rta_type = IFA_ADDRESS;
	rtap->rta_len = sizeof(struct rtattr) + 4;
	inet_pton(AF_INET, dsts,
	((char *)rtap) + sizeof(struct rtattr));
	ifal += rtap->rta_len;


	// add second attrib:
	// set ifc index and increment the size
	rtap = (struct rtattr *) (((char *)rtap)
	+ rtap->rta_len);
	rtap->rta_type = IFA_LOCAL;
	rtap->rta_len = sizeof(struct rtattr) + 4;
	inet_pton(AF_INET, dsts,
	((char *)rtap) + sizeof(struct rtattr));
	ifal += rtap->rta_len;

	// setup the NETLINK header
	req.nl.nlmsg_len = NLMSG_LENGTH(ifal);
	/* TODO: test with NLM_F_APPEND */
	req.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_APPEND;
	req.nl.nlmsg_type = RTM_NEWADDR;

	// setup the service header (struct rtmsg)
	req.rt.ifa_family = AF_INET;
	req.rt.ifa_prefixlen = 32; /*hardcoded*/
	req.rt.ifa_flags = IFA_F_PERMANENT;
	req.rt.ifa_scope = RT_SCOPE_HOST;
	/*TODO: think how can you find out the ifa_index */
	req.rt.ifa_index = 3;

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

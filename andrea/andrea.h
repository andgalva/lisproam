/*
 * andrea.h
 *
 *  Created on: Oct 3, 2013
 *      Author: andrea
 */

#ifndef ANDREA_H_
#define ANDREA_H_

#include "../lispd.h"
#include "../lispd_mapping.h"
#include "../lispd_map_notify.h"
#include "../lispd_iface_list.h"
#include "../patricia/patricia.h"

#include <netinet/in.h>
#include "radius.h"
#include "vector.h"
#include "dhcp.h"

#define RADIUS_PORT       			1812
#define RADIUS_CODE_ACCESS_REQUEST	1
#define RADIUS_CODE_ACCESS_ACCEPT	2
#define DHCP_PORT					67

#define RADIUS_SERVER_IP			"84.88.81.48"
#define HOME_NETWORK				"10.1.1.0"
#define HOME_ROUTER					"10.1.1.254"
#define HOME_NETMASK				"255.255.255.0"
#define HOME_BROADCAST				"10.1.1.255"

typedef struct user_info {
	char username[50];
	char eid[INET_ADDRSTRLEN];
	char mac[20];

	char ms_address[INET_ADDRSTRLEN];
	char ms_key[50];
	uint64_t ms_nonce;

	int wlan_id;
} user_info;

extern vector USERS_INFO;
static int WLAN_ID = 6;
extern char WLAN_INTERFACE[50];

void andrea_add_wlan(user_info *user);
void andrea_request_map_server(user_info *user);
void andrea_send_map_register(user_info *user);
void andrea_check_map_notify(lispd_pkt_map_notify_t *pkt);
void create_mock_user(int signum);

#endif /* ANDREA_H_ */

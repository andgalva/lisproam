/*
 * andrea.c
 *
 *  Created on: Oct 8, 2013
 *      Author: andrea
 */


#include "andrea.h"

#include <bits/sockaddr.h>
#include <asm/types.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

void user_info_print(user_info *ui)
{
	printf("\t --- User info ---\n");

	printf("\t> username: %s \n", strlen(ui->username)>0 ? ui->username : "-");
	printf("\t> eid: %s \n", strlen(ui->eid)>0 ? ui->eid : "-");
	printf("\t> mac: %s \n", strlen(ui->mac)>0 ? ui->mac : "-");
	printf("\t> wlan id: %d \n", ui->wlan_id!=NULL ? ui->wlan_id : -1);
	printf("\t> MS address: %s \n", strlen(ui->ms_address)>0 ? ui->ms_address : "-");
	printf("\t> MS key: %s \n", strlen(ui->ms_key)>0 ? "(assigned)" : "-");

	printf("\t --- end ---\n");
}

void andrea_add_wlan(user_info *user)
{

	printf("\n\t=> Adding configuration for user '%s' (%s)\n", user->username, user->mac);

	// ADDRESS a.b.c.N <- we care about N

	int i=0, n=0;
	char eid_base[15], N_str[3];

	while(n<3) {
		if (user->eid[i] == '.')
			n++;
		i++;
	}
	strncpy(eid_base, user->eid, i);
	eid_base[i] = '\0';

	strncpy(N_str, user->eid+i, strlen(user->eid)-i);
	N_str[strlen(user->eid)-i] = '\0';

	int N = atoi(N_str);
	int dgw_N, broad_N, network_N;

	if (N % 4 == 0 || N % 4 == 3)
	{
		printf("EID %s can not be used with a /30 netmask", user->eid);
		return;
	}
	else
	{
		if (N % 2 == 0)
		{
			network_N = N-2;
			dgw_N = N-1;
			broad_N = N+1;
		}
		else
		{
			network_N = N-1;
			dgw_N = N+1;
			broad_N = N+2;
		}
	}

	char eid_network[20], eid_dgw[20], eid_broad[20];
	sprintf(eid_network,"%s%d",eid_base, network_N);
	sprintf(eid_dgw,"%s%d",eid_base, dgw_N);
	sprintf(eid_broad,"%s%d",eid_base, broad_N);

	char command[150];
	int curr_wlan = WLAN_ID;
	WLAN_ID++;

	sprintf(command, "ifconfig %s:%d %s netmask 255.255.255.252 broadcast %s",
			WLAN_INTERFACE, curr_wlan, eid_dgw, eid_broad);
	printf("EXECUTE: %s\n", command);
	system(command);

	/*system("cp /etc/dnsmasq.conf /etc/dnsmasq.conf.`date +%F`_`date +%T`");
	FILE* fp;
	fp = fopen("/etc/dnsmasq.conf", "a+");
	fprintf(fp, "\n# %s (%s/30) \n"
			"dhcp-host=%s,set:%s,%s \n"
			"dhcp-option=tag:%s,3,%s \n"
			"dhcp-option=tag:%s,54,%s \n"
			"dhcp-option=tag:%s,1,255.255.255.252 \n"
			"dhcp-option=tag:%s,28,%s \n"
			"dhcp-option=tag:%s,6,%s \n"
			"dhcp-option=tag:%s,15,%s \n",
			user->username,	eid_network,
			user->mac, user->username, user->eid,
			user->username, eid_dgw,
			user->username, eid_dgw,
			user->username,
			user->username, eid_broad,
			user->username, eid_dgw,
			user->username, user->username);
	fclose(fp);*/

	sprintf(command, "/etc/init.d/dnsmasq reload");
	printf("EXECUTE: %s\n", command);
	system(command);

	user->wlan_id = curr_wlan;

	user_info_print(user);

	//free(user);
}

void andrea_request_map_server(user_info *user)
{
	/* -> SEND MAP-REQUEST FOR THIS EID

	lispd_mapping_elt *lme = malloc(sizeof(lispd_mapping_elt));
	get_lisp_addr_from_char(user->eid, &(lme->eid_prefix));
	//get_lisp_addr_from_char("10.1.2.121", &(lme->eid_prefix));
	lme->eid_prefix_length = 32;
	*/

	lisp_addr_t *home_eid = &(get_head_interface_list()->iface->head_mappings_list->mapping->eid_prefix);

	lisp_addr_t  *dest_eid = malloc(sizeof(lisp_addr_t));;
	get_lisp_addr_from_char(user->eid, dest_eid);

	uint64_t nonce;

	lispd_mapping_elt *mapping = (lispd_mapping_elt *)malloc(sizeof(lispd_mapping_elt));
	mapping->eid_prefix =  *dest_eid;
	mapping->eid_prefix_length = 32;
	mapping->iid = -1;
	mapping->locator_count = 0;
	mapping->head_v4_locators_list = NULL;
	mapping->head_v6_locators_list = NULL;

	build_and_send_map_request_msg(
			mapping,
			home_eid,
			get_map_resolver(),
			1,
			0,
			0,
			0,
			&nonce);

	user->ms_nonce = nonce;

/*
	free(source_eid);
	free(eid_prefix);


	// Since the Map-Request in not working, SHORTCUT
	strcpy(user->ms_address, "84.88.81.2");
	andrea_send_map_register(user);*/
}

void andrea_send_map_register(user_info *user)
{
	// TODO modify Map-Server's entries, marking which ones are "foreign"
	add_map_server(user->ms_address, 1, user->ms_key, 1);

	char eid_with_mask[INET_ADDRSTRLEN +3];
	sprintf(eid_with_mask,"%s/32",user->eid);
	// We need to pass the EID with the "/MASK"
	add_database_mapping(eid_with_mask, -1, get_head_interface_list()->iface->iface_name, 1, 100, 1, 100);

	/*
	lisp_addr_t  *eid_prefix = malloc(sizeof(lisp_addr_t));;
	get_lisp_addr_from_char(user->eid, eid_prefix);

	lispd_mapping_elt *mapping = (lispd_mapping_elt *)malloc(sizeof(lispd_mapping_elt));
	mapping->eid_prefix =  *eid_prefix;
	mapping->eid_prefix_length = 32;
	mapping->iid = -1;
	mapping->locator_count = 1;

	lisp_addr_t *home_eid = &(get_head_interface_list()->iface->head_mappings_list->mapping->eid_prefix);

	mapping->head_v6_locators_list = NULL;

	mapping->head_v4_locators_list = (lispd_locators_list *) malloc(sizeof(lispd_locators_list));
	mapping->head_v4_locators_list->locator = (lispd_locator_elt *) malloc(sizeof(lispd_locator_elt));
	//mapping->head_v4_locators_list->locator->locator_addr = (lisp_addr_t *) malloc(sizeof(lisp_addr_t));
	//get_lisp_addr_from_char("147.83.42.175", mapping->head_v4_locators_list->locator->locator_addr);
    lispd_mapping_elt *home_mapping = lookup_eid_in_db(*home_eid);
	mapping->head_v4_locators_list->locator->locator_addr =
			home_mapping->head_v4_locators_list->locator->locator_addr;
	mapping->head_v4_locators_list->next = NULL; // !!!

	mapping->head_v4_locators_list->locator->priority = 1;
	mapping->head_v4_locators_list->locator->weight = 100;
	mapping->head_v4_locators_list->locator->mpriority = 255;
	mapping->head_v4_locators_list->locator->mweight = 1;
	mapping->head_v4_locators_list->locator->state = malloc(sizeof(uint8_t));
	*mapping->head_v4_locators_list->locator->state = 1;
	mapping->head_v4_locators_list->locator->data_packets_in = 0;
	mapping->head_v4_locators_list->locator->data_packets_out = 0;
	mapping->head_v4_locators_list->locator->locator_type = LOCAL_LOCATOR;
	mapping->head_v4_locators_list->locator->extended_info = NULL;

	// add the new host to the local DB
	add_mapping_to_db(mapping);
	*/

	// send a Map-Register for every EID-prefix in the local DB
	map_register(NULL, NULL);
}

void andrea_check_map_notify(lispd_pkt_map_notify_t *pkt) {
	//pkt = (lispd_pkt_map_notify_t *) pkt;

	/* CHECK if it's about a MOVED host */

    lispd_pkt_mapping_record_t          *record;
    lispd_pkt_mapping_record_locator_t  *record_locator;
    int record_count = pkt->record_count;
    int locator_count;
    int eid_afi, loc_afi;
    int eid_length, locator_length;
    int i, j;

    uint8_t *eid;
    uint8_t *locator;

    record = (lispd_pkt_mapping_record_t *)CO(pkt, sizeof(lispd_pkt_map_notify_t));
    for (i=0; i < record_count; i++)
    {
        eid_length = sizeof(lispd_pkt_mapping_record_t);
        eid_afi = lisp2inetafi(ntohs(record->eid_prefix_afi));
        if (eid_afi < 0)
            continue;
        switch (eid_afi) {
        case AF_INET:
			eid = (uint8_t * )ntohl(CO(record, eid_length));

            eid_length += sizeof(struct in_addr);
            break;
        case AF_INET6:
            eid_length += sizeof(struct in6_addr);

            // TODO

            break;
        default:
            continue;
        }

        locator_count = record->locator_count;
        record_locator = (lispd_pkt_mapping_record_locator_t *)CO(record, eid_length);
        for ( j=0 ; j<locator_count ; j++)
        {
            locator_length = sizeof(lispd_pkt_mapping_record_locator_t);
            loc_afi = lisp2inetafi(ntohs(record_locator->locator_afi));
            switch (loc_afi) {

            case AF_INET:

				locator = (uint8_t * )ntohl(CO(record_locator, locator_length));

                char eid_str[INET_ADDRSTRLEN];
				sprintf(eid_str, "%u.%u.%u.%u", eid[0], eid[1], eid[2], eid[3]);

                char locator_str[INET_ADDRSTRLEN];
				sprintf(locator_str, "%u.%u.%u.%u", locator[0], locator[1], locator[2], locator[3]);

				lisp_addr_t *home_eid = &(get_head_interface_list()->iface->head_mappings_list->mapping->eid_prefix);

				lispd_mapping_elt *home_mapping = lookup_eid_in_db(*home_eid);
			    char home_locator_str[INET_ADDRSTRLEN];
				strcpy(home_locator_str,
						get_char_from_lisp_addr_t(*home_mapping->head_v4_locators_list->locator->locator_addr));


				user_info *ui = vector_search_eid(&USERS_INFO, eid_str);
				if (ui != NULL &&	strcmp(locator_str, home_locator_str) != 0)
				{
					// This user (eid) moved! => Send SMR
					lispd_log_msg(LISP_LOG_INFO,
							"\n\n\t=> !!!!!!!!!!!!!!!!"
							"\n\tUser '%s' moved to a new locator: %s"
							"\n\t!!!!!!!!!!!!!!!! <=\n\n", ui->username, locator_str);


					// we must "mark" the interface in order to send SMRs
				    lispd_iface_list_elt *iface_list = get_head_interface_list();

					lispd_mapping_elt *moved_host_mapping = (lispd_mapping_elt *)malloc(sizeof(lispd_mapping_elt));
					lisp_addr_t host_eid;
					get_lisp_addr_from_char(eid_str, &host_eid);
					moved_host_mapping->eid_prefix =  host_eid;
					moved_host_mapping->eid_prefix_length = 32;
					moved_host_mapping->iid = -1;
					moved_host_mapping->locator_count = 0;
					moved_host_mapping->head_v4_locators_list = NULL;
					moved_host_mapping->head_v6_locators_list = NULL;

				    //add_mapping_to_interface(iface_list, moved_host_mapping, AF_INET);

				    iface_list->iface->status_changed = TRUE;
				    // send SMRs to changed interfaces
					init_smr(NULL, NULL);

					// remove moved_host_mapping from local DB
					lisp_addr_t moved_eid;
					get_lisp_addr_from_char(ui->eid, &moved_eid);
					del_mapping_entry_from_db(moved_eid, 32);
					// TODO I have to remove it also from the interface's mapping! (There's no function)

					// add new moved_host_mapping to Map-Cache -> send a Map-Request ?
					int nonce;
					build_and_send_map_request_msg(
							moved_host_mapping,
							home_eid,
							get_map_resolver(),
							1,
							0,
							0,
							0,
							&nonce);
				}

                locator_length = locator_length + sizeof(struct in_addr);
                break;
            case AF_INET6:
                locator_length = locator_length + sizeof(struct in6_addr);

                // TODO

                break;
            default:
                continue;
            }

            record_locator = (lispd_pkt_mapping_record_locator_t *)CO(record_locator, locator_length);
        }

        record = (lispd_pkt_mapping_record_t *)record_locator;
    }

}

void create_mock_user(int signum) {

	// SAME PROCEDURE AS WE RECEIVE RADIUS ACCEPT

	user_info *user = (user_info*) malloc(sizeof(user_info));
	strcpy(user->username, "alice");
	strcpy(user->mac, "00:0d:88:65:5a:5d");
	strcpy(user->eid, "10.1.2.121");
	strcpy(user->ms_address, "84.88.81.2");
	strcpy(user->ms_key, "lispmob");

	lispd_log_msg(LISP_LOG_INFO, "\n\t=> Authentication completed for user '%s'!", user->username);

	vector_add(&USERS_INFO, user);

	andrea_add_wlan(user);

	andrea_request_map_server(user);

}

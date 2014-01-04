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
	if (ui->foreign == 1)
	{
		printf("\t> FOREIGN user \n");
		printf("\t\t> wlan id: %d \n", ui->wlan_id!=NULL ? ui->wlan_id : -1);
		printf("\t\t> MS address: %s \n", strlen(ui->ms_address)>0 ? ui->ms_address : "-");
		printf("\t\t> MS key: %s \n", strlen(ui->ms_key)>0 ? "(assigned)" : "-");
	}
	else
	{
		printf("\t> HOME user \n");
	}
	printf("\t --- end ---\n");
}

void andrea_add_local_configuration(user_info *user)
{
	char command[150];

	// Check if user is HOME or FOREIGN user
	// TODO To be fixed! Now the check is done assuming ALWAYS a /24 EID-prefix!
	lisp_addr_t *home_eid = &(get_head_interface_list()->iface->head_mappings_list->mapping->eid_prefix);
	char home_eid_str[INET_ADDRSTRLEN];
	strcpy(home_eid_str, get_char_from_lisp_addr_t(*home_eid));

	int i=0, n=0;
	while(n<3)
	{
		if (home_eid_str[i] != user->eid[i])
			break;
		if (home_eid_str[i] == '.')
			n++;
		i++;
	}

	if (n != 3)
	{
		/* FOREIGN USER */

		user->foreign = 1;

		lispd_log_msg(LISP_LOG_INFO, "\tLISProam: Adding wlan configuration for foreign user '%s' (%s)\n",
				user->username, user->mac);

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
			lispd_log_msg(LISP_LOG_ERR, "\tLISProam: EID %s can not be used with a /30 netmask\n", user->eid);
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

		int curr_wlan = WLAN_ID;
		WLAN_ID++;

		// WLAN

		sprintf(command, "ifconfig %s:%d %s netmask 255.255.255.252 broadcast %s",
				WLAN_INTERFACE, curr_wlan, eid_dgw, eid_broad);
		lispd_log_msg(LISP_LOG_INFO, "\tLISProam: Added interface %s:%d\n", WLAN_INTERFACE, curr_wlan);
		system(command);

		// DHCP

		if (andrea_check_dhcp_entry(user) == GOOD)
			lispd_log_msg(LISP_LOG_INFO, "\tLISProam: Foreign user '%s' already stored in DHCP Server\n", user->username);
		else
		{
			andrea_add_dhcp_entry_foreign(user, eid_network, eid_dgw, eid_broad);

			sprintf(command, "/etc/init.d/dnsmasq reload");
			lispd_log_msg(LISP_LOG_INFO, "\tLISProam: Reloading DHCP Server\n");
			system(command);
		}

		user->wlan_id = curr_wlan;
	}
	else
	{
		/* HOME USER */

		user->foreign = 0;

		if (andrea_check_dhcp_entry(user) == GOOD)
			lispd_log_msg(LISP_LOG_INFO, "\tLISProam: Home user '%s' already stored in DHCP Server\n", user->username);
		else
		{
			andrea_add_dhcp_entry_home(user);

			sprintf(command, "/etc/init.d/dnsmasq reload");
			lispd_log_msg(LISP_LOG_INFO, "\tLISProam: Reloading DHCP Server\n");
			system(command);
		}

		user->wlan_id = -1;
	}

}

void andrea_add_dhcp_entry_home(user_info *user)
{
	// Backup
	system("cp /etc/dnsmasq.conf /etc/dnsmasq.conf.`date +%F`_`date +%T`");

	FILE* fp;
	fp = fopen("/etc/dnsmasq.conf", "a+");

	lispd_log_msg(LISP_LOG_INFO, "\tLISProam: Adding DHCP entry for home user '%s'\n", user->username);
	fprintf(fp, "\n# %s START (home) \n"
			"dhcp-host=%s,set:home,%s \n"
			"# %s END (home) \n",
			user->username,
			user->mac, user->eid,
			user->username);
	fclose(fp);
}

void andrea_add_dhcp_entry_foreign(user_info *user, char *eid_network, char *eid_dgw, char *eid_broad)
{
	// Backup
	system("cp /etc/dnsmasq.conf /etc/dnsmasq.conf.`date +%F`_`date +%T`");

	FILE* fp;
	fp = fopen("/etc/dnsmasq.conf", "a+");

	lispd_log_msg(LISP_LOG_INFO, "\tLISProam: Adding DHCP entry for foreign user '%s'\n", user->username);
	fprintf(fp, "\n# %s START (%s/30) \n"
			"dhcp-host=%s,set:%s,%s \n"
			"dhcp-option=tag:%s,3,%s \n"
			"dhcp-option=tag:%s,54,%s \n"
			"dhcp-option=tag:%s,1,255.255.255.252 \n"
			"dhcp-option=tag:%s,28,%s \n"
			"dhcp-option=tag:%s,6,%s \n"
			"dhcp-option=tag:%s,15,%s \n"
			"# %s END (%s/30) \n",
			user->username,	eid_network,
			user->mac, user->username, user->eid,
			user->username, eid_dgw,
			user->username, eid_dgw,
			user->username,
			user->username, eid_broad,
			user->username, eid_dgw,
			user->username, user->username,
			user->username,	eid_network);
	fclose(fp);
}

int andrea_check_dhcp_entry(user_info *user)
{
	FILE *fp;
	fp = fopen("/etc/dnsmasq.conf", "r");

	int len;
	char line[4096];

	int check = 0;
	char user_start[100];
	sprintf(user_start,"# %s START", user->username);

	if (fp == NULL)
		return BAD;

	while (fgets(line, sizeof(line), fp))
	{
		len = strlen(line);
		if (len && (line[len-1] != '\n'))
			{}
		else
		{
			if (strstr(line, user_start) != NULL)
				check = 1;
			if (check==1 && strstr(line, user->mac) != NULL)
			{
				check = 2;
				break;
			}
		}
	}

	fclose(fp);

	if (check==2)
		return GOOD;
	else
		return BAD;
}

void andrea_remove_dhcp_entry(user_info *user)
{
	FILE *old_fp;
	old_fp = fopen("/etc/dnsmasq.conf", "r");
	FILE *new_fp;
	new_fp = fopen("/etc/dnsmasq.conf.tmp", "w");

	int line_num = 0;
	int len;
	char line[4096];

	int delete = 0;
	char user_start[100], user_end[100];
	sprintf(user_start,"# %s START", user->username);
	sprintf(user_end,"# %s END", user->username);

	if (old_fp == NULL)
		return;

	while (fgets(line, sizeof(line), old_fp))
	{
		len = strlen(line);
		if (len && (line[len-1] != '\n'))
			{}
		else
		{
			if (strstr(line, user_start) != NULL)
				delete = 1;
			if (strstr(line, user_end) != NULL)
				delete = 0;

			if (delete == 1)
			{
				// skip line
			}
			else
			{
				fputs(line, new_fp);
			}

			line_num++;
		}
	}

	remove("/etc/dnsmasq.conf");
	rename("/etc/dnsmasq.conf.tmp", "/etc/dnsmasq.conf");
	fclose(old_fp);
	fclose(new_fp);
}

void andrea_send_map_request(user_info *user)
{
	/* -> SEND MAP-REQUEST FOR THIS EID */

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

	lispd_log_msg(LISP_LOG_INFO, "\tLISProam: Requesting Map-Server for user '%s' (%s)\n", user->username, user->eid);

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
}

void andrea_send_map_register(user_info *user)
{

	if (user->foreign == 1)
	{
		char eid_with_mask[INET_ADDRSTRLEN +3];
		sprintf(eid_with_mask,"%s/32",user->eid);
		lispd_log_msg(LISP_LOG_INFO, "\tLISProam: Adding new user's EID %s to Local DB\n", eid_with_mask);
		// We need to pass the EID with the "/MASK"
		add_database_mapping(eid_with_mask, -1, get_head_interface_list()->iface->iface_name, 1, 100, 1, 100);
	}

	lispd_log_msg(LISP_LOG_INFO, "\tLISProam: Sending Map-Register for new EID (%s)\n", user->eid);
	// /map_register(NULL, NULL) -> sends a Map-Register for every EID-prefix in the local DB

	// we send a Map-Register ONLY for the new EID
	lisp_addr_t user_eid;
	get_lisp_addr_from_char(user->eid, &user_eid);
	lispd_mapping_elt *mapping = new_local_mapping(user_eid, 32, -1);
	lispd_locator_elt *locator = new_local_locator (get_head_interface_list()->iface->ipv4_address,&(get_head_interface_list()->iface->status),
						1, 100, 255, 0, get_head_interface_list()->iface->out_socket_v4);

	add_locator_to_mapping (mapping,locator);
	calculate_balancing_vectors (mapping,&((lcl_mapping_extended_info *)mapping->extended_info)->outgoing_balancing_locators_vecs);

	build_and_send_map_register_msg(mapping);
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
							"\n\tLISProam: !!! User '%s' moved to a new locator: %s !!!\n", ui->username, locator_str);


					// we must "mark" the interface in order to send SMRs
				    lispd_iface_list_elt *iface_list = get_head_interface_list();
				    iface_list->iface->status_changed = TRUE;


				    // if the moved user is HOME, we have to add the /32 mappng to the local DB, for the SMRs
				    if (ui->foreign == 0)
				    {
						char eid_with_mask[INET_ADDRSTRLEN +3];
						sprintf(eid_with_mask,"%s/32",ui->eid);
						add_database_mapping(eid_with_mask, -1, get_head_interface_list()->iface->iface_name, 1, 100, 1, 100);
				    }

				    // send SMRs to all CNs
					init_smr(NULL, NULL);

					// remove moved_host_mapping from local DB
					lisp_addr_t moved_eid;
					get_lisp_addr_from_char(ui->eid, &moved_eid);
					del_mapping_entry_from_db(moved_eid, 32);
					// TODO I have to remove it also from the interface's mapping! (There's no function)


					// add new moved_host_mapping to Map-Cache -> send a Map-Request (?)
					lispd_mapping_elt *moved_host_mapping = (lispd_mapping_elt *)malloc(sizeof(lispd_mapping_elt));
					lisp_addr_t host_eid;
					get_lisp_addr_from_char(eid_str, &host_eid);
					moved_host_mapping->eid_prefix =  host_eid;
					moved_host_mapping->eid_prefix_length = 32;
					moved_host_mapping->iid = -1;
					moved_host_mapping->locator_count = 0;
					moved_host_mapping->head_v4_locators_list = NULL;
					moved_host_mapping->head_v6_locators_list = NULL;
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

					// We don't delete the entry from the vector. We should set a TTL (+ probing) to each entry.
					//vector_delete(&USERS_INFO, ui->eid);

					if (ui->foreign == 1)
						andrea_remove_wlan(ui);
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

	andrea_add_local_configuration(user);

	andrea_send_map_request(user);

}

void andrea_clean() {

	andrea_clean_dhcp();

	int i;
	for (i = 0; i < vector_count(&USERS_INFO); i++) {
		struct user_info *ui = (struct user_info *) vector_get(&USERS_INFO, i);
		if (ui != NULL && ui->foreign == 1 && ui->wlan_id != NULL && ui->wlan_id > 1)
		{
			andrea_remove_wlan(ui);
		}
	}

	vector_free(&USERS_INFO);
}

void andrea_remove_wlan(user_info *ui)
{
	char command[100];
	sprintf(command, "ifconfig %s:%d down", WLAN_INTERFACE, ui->wlan_id);
	lispd_log_msg(LISP_LOG_INFO, "\tLISProam: Deleted interface %s:%d\n", WLAN_INTERFACE, ui->wlan_id);
	system(command);
	// reset wlan_id
	ui->wlan_id = -1;
}

void andrea_clean_dhcp()
{
	FILE *old_fp;
	old_fp = fopen("/etc/dnsmasq.conf", "r");
	FILE *new_fp;
	new_fp = fopen("/etc/dnsmasq.conf.tmp", "w");

	int line_num = 0;
	int len;
	char line[4096];

	int delete = 0;
	char lisproam_start[100];
	sprintf(lisproam_start,"# LISProam START");

	if (old_fp == NULL)
		return;

	while (fgets(line, sizeof(line), old_fp))
	{
		len = strlen(line);
		if (len && (line[len-1] != '\n'))
			{}
		else
		{
			fputs(line, new_fp);

			if (strstr(line, lisproam_start) != NULL)
				break;

			line_num++;
		}
	}

	remove("/etc/dnsmasq.conf");
	rename("/etc/dnsmasq.conf.tmp", "/etc/dnsmasq.conf");
	fclose(old_fp);
	fclose(new_fp);
}

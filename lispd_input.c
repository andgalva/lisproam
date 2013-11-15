/*
 * lispd_input.c
 *
 * This file is part of LISP Mobile Node Implementation.
 *
 * Copyright (C) 2012 Cisco Systems, Inc, 2012. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please send any bug reports or fixes you make to the email address(es):
 *    LISP-MN developers <devel@lispmob.org>
 *
 * Written or modified by:
 *    Alberto Rodriguez Natal <arnatal@ac.upc.edu>
 */


#include "lispd_input.h"
#include "lispd_map_reply.h"
#include "andrea/andrea.h"
#include <ctype.h>

void process_input_packet(int fd,
                          int afi,
                          int tun_receive_fd)
{
    uint8_t             *packet = NULL;
    int                 length = 0;
    uint8_t             ttl = 0;
    uint8_t             tos = 0;

    struct lisphdr      *lisp_hdr = NULL;
    struct iphdr        *iph = NULL;
    struct ip6_hdr      *ip6h = NULL;
    struct udphdr       *udph = NULL;


    if ((packet = (uint8_t *) malloc(MAX_IP_PACKET))==NULL){
        lispd_log_msg(LISP_LOG_ERR,"process_input_packet: Couldn't allocate space for packet: %s", strerror(errno));
        return;
    }

    memset(packet,0,MAX_IP_PACKET);
    
    if (get_data_packet (fd,
                         afi,
                         packet,
                         &length,
                         &ttl,
                         &tos) == BAD){
        lispd_log_msg(LISP_LOG_DEBUG_2,"process_input_packet: get_data_packet error: %s", strerror(errno));
        free(packet);
        return;
    }

    if(afi == AF_INET){
        /* With input RAW UDP sockets in IPv4, we get the whole external IPv4 packet */
        udph = (struct udphdr *) CO(packet,sizeof(struct iphdr));
    }else{
        /* With input RAW UDP sockets in IPv6, we get the whole external UDP packet */
        udph = (struct udphdr *) packet;
    }
    
    /* With input RAW UDP sockets, we receive all UDP packets, we only want lisp data ones */
    if(ntohs(udph->dest) != LISP_DATA_PORT
    		&& ntohs(udph->source) != RADIUS_PORT
    		&& ntohs(udph->dest) != LISP_CONTROL_PORT)
    {
        free(packet);
        //lispd_log_msg(LISP_LOG_DEBUG_3,"INPUT (No LISP data): UDP dest: %d ",ntohs(udph->dest));
        return;
    }

    /* LISP packet (normal lispmob) */
    if(ntohs(udph->dest) == LISP_DATA_PORT)
    {
		lisp_hdr = (struct lisphdr *) CO(udph,sizeof(struct udphdr));

		length = length - sizeof(struct udphdr) - sizeof(struct lisphdr);

		iph = (struct iphdr *) CO(lisp_hdr,sizeof(struct lisphdr));

		lispd_log_msg(LISP_LOG_DEBUG_3,"INPUT (4341): Inner src: %s | Inner dst: %s ",
					  get_char_from_lisp_addr_t(extract_src_addr_from_packet((char *)iph)),
					  get_char_from_lisp_addr_t(extract_dst_addr_from_packet((char *)iph)));

		if (iph->version == 4) {

			if(ttl!=0){ /*XXX It seems that there is a bug in uClibc that causes ttl=0 in OpenWRT. This is a quick workaround */
				iph->ttl = ttl;
			}
			iph->tos = tos;

			/* We need to recompute the checksum since we have changed the TTL and TOS header fields */
			iph->check = 0; /* New checksum must be computed with the checksum header field with 0s */
			iph->check = ip_checksum((uint16_t*) iph, sizeof(struct iphdr));

		}else{
			ip6h = ( struct ip6_hdr *) iph;

			if(ttl!=0){ /*XXX It seems that there is a bug in uClibc that causes ttl=0 in OpenWRT. This is a quick workaround */
				ip6h->ip6_hops = ttl; /* ttl = Hops limit in IPv6 */
			}

			IPV6_SET_TC(ip6h,tos); /* tos = Traffic class field in IPv6 */
		}

	    /* LISP packets must be forwarded */
	    if ((write(tun_receive_fd, iph, length)) < 0){
	        lispd_log_msg(LISP_LOG_DEBUG_2,"lisp_input: write error: %s\n ", strerror(errno));
	    }
    }




    /* RADIUS incoming packet - START */
    if (ntohs(udph->source) == RADIUS_PORT)
    {
		struct radius_packet *rpacket = (struct radius_packet *) CO(udph,sizeof(struct udphdr));
		lispd_log_msg(LISP_LOG_INFO, "\tIncoming RADIUS Access-Request, code = %u", rpacket->code );

		if (rpacket->code == RADIUS_CODE_ACCESS_ACCEPT)
		{

			uint8_t *eid;
			char eid_str[20]; memset(eid_str, 0, sizeof(eid_str));
			char lisp_key[50]; memset(lisp_key, 0, sizeof(lisp_key));
			char username[50]; memset(username, 0, sizeof(username));

			struct radius_attribute *rattribute = rpacket->attrs;
			while(rattribute != NULL && rattribute->type != 0)
			{
				//lispd_log_msg(LISP_LOG_INFO, "RADIUS attribute type = %d", rattribute->type);

				switch(rattribute->type) {
					// Here we have the User-Name (type=1) in rattribute
					case 1: ;
						strncpy(username, rattribute->value, rattribute->length -2);
						username[rattribute->length -2] = '\0';

						lispd_log_msg(LISP_LOG_INFO, "\t\tUser-Name: %s", username);

						break;

					// Here we have the Framed-IP-Address (type=8) in rattribute
					case 8: ;
						eid = (uint8_t * )ntohl(rattribute->value);

						sprintf(eid_str, "%u.%u.%u.%u", eid[0], eid[1], eid[2], eid[3]);

						lispd_log_msg(LISP_LOG_INFO, "\t\tFramed-IP-Address: %s", eid_str);

						break;

					// Here we have the Reply-Message (type=18) in rattribute
					case 18: ;
						/*
						 * N.B. 'length' is related to the whole packet:
						 * the string is 'length-2' long because we don't consider
						 * 'type' and 'size' (each one is 1 byte)
						 */
						strncpy(lisp_key, rattribute->value, rattribute->length -2);
						lisp_key[rattribute->length -2] = '\0';

						lispd_log_msg(LISP_LOG_INFO, "\t\tReply-Message: %s", lisp_key);

						break;

					default: break;
				}

				// if I have everything I need...
				if (strlen(username) != 0 && strlen(lisp_key) != 0 && strlen(eid_str) != 0)
					break;
				else // go on reading...
					rattribute = (struct radius_attribute *) CO(rattribute, rattribute->length);
			}

			if (strlen(username) != 0 && strlen(lisp_key) != 0 && strlen(eid_str) != 0)
			{
				user_info *user = vector_search_username(&USERS_INFO, username);

				strcpy(user->ms_key, lisp_key);
				strcpy(user->eid, eid_str);

				lispd_log_msg(LISP_LOG_INFO, "\n\t=> Authentication completed for user '%s'!", username);

				andrea_add_wlan(user);

				andrea_request_map_server(user);

				// when reply is received ->
				// -> READ RLOC(S) IN MAP-REPLY WHERE NONCE MATCHES
				// -> SEND MAP-REGISTER TO THAT RLOC

				// ADD FUNCTIONS FOR "PREVIOUS XTR"
				// -> IF I RECEIVE A MAP-NOTIFY
				// -> IF THE EID IS ONE OF MY USERS
				// -> IF THE RLOCS ARE DIFFERENT THAN MINE
				// ... HE MOVED
				// -> SEND SMRs
				// -> DELETE USER FROM VECTOR
				// -> DELETE USER FROM DNSMASQ (?)
				// -> DELETE wlan0:X
				// -> ?

				// How do SMRs work on lispmob?
				// Do i have to modify Map-Cache (adding MN <-> CNs)?
			}
		}
    } /* RADIUS incoming packet - END */


    /* LISP control packet */
    if(ntohs(udph->dest) == LISP_CONTROL_PORT)
    {
    	lispd_pkt_map_reply_t *pkt = (struct lispd_pkt_map_request_t *) CO(udph,sizeof(struct udphdr));
    	/* Map-Reply */
		if (pkt->type == LISP_MAP_REPLY)
		{
			/* CHECK if it's the one we are waiting for */
			user_info *user = (user_info *) vector_search_nonce(&USERS_INFO, pkt->nonce);
			if (user != NULL)
			{
				strcpy(user->ms_address, get_char_from_lisp_addr_t(extract_src_addr_from_packet(packet)));
				user->ms_nonce = -1; // After we used it, we don't need it anymore!
				user_info_print(user);

				andrea_send_map_register(user);
			}
		}
    	/* Map-Notify */
		else if (pkt->type == LISP_MAP_NOTIFY)
		{
			andrea_check_map_notify(pkt);
		}
    }

    free(packet);
}


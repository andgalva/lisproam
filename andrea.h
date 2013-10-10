/*
 * andrea.h
 *
 *  Created on: Oct 3, 2013
 *      Author: andrea
 */

#ifndef ANDREA_H_
#define ANDREA_H_

#include <netinet/in.h>
#include "radius.h"

#define RADIUS_PORT       1812
#define DHCP_PORT			53

static int ID_INTERFACE = 100;
static int NL_SEQNUM = 1;

void create_vlan();

#endif /* ANDREA_H_ */

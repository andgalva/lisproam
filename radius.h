/*
 * radius.h
 *
 *  Created on: Oct 3, 2013
 *      Author: andrea
 */

#ifndef RADIUS_H_
#define RADIUS_H_

#define RADIUS_AUTHENTICATOR_LENGTH 16
#define RADIUS_PASSWORD_BLOCK_SIZE 16
#define RADIUS_HEADER_LENGTH 20

#define RADIUS_MAX_SIZE 4096
#define RADIUS_MAX_ATTRIBUTE_SIZE 253

typedef uint8_t radius_authenticator[RADIUS_AUTHENTICATOR_LENGTH];


typedef struct radius_packet
{
     /// The RADIUS message type code
     uint8_t  code;

     /// RADIUS identifier
     uint8_t  identifier;

     /// Total length of the packet, including the header
     uint16_t length;

     /// RADISU authenticator
     uint8_t  authenticator[RADIUS_AUTHENTICATOR_LENGTH];

     /// All attributes in serial packed format
     uint8_t  attrs[RADIUS_MAX_SIZE - RADIUS_HEADER_LENGTH];

} radius_packet_t;

typedef struct radius_attribute
{
     /// Attribute number
     uint8_t  type;

     /// Total length, including header
     uint8_t  length;

     /// Value of the attribute. May have some internal structure, such as for VSAs etc
     uint8_t  value[RADIUS_MAX_ATTRIBUTE_SIZE];

} radius_attribute_t;

#endif /* RADIUS_H_ */

/* SOURCE: https://github.com/ethersex/ethersex/blob/master/protocols/bootp/bootphdr.h */

#ifndef DHCP_H_
#define DHCP_H_

#define BP_CHADDR_LEN 16
#define BP_SNAME_LEN 64
#define BP_FILE_LEN 128
#define BP_VEND_LEN 64
#define BP_MINPKTSZ 300 /* to check sizeof(struct bootp) */

/* Tell the server to broadcast to reach me flag */
#define BPFLAG_BROADCAST ( 1 << 15 )

typedef struct dhcp_packet
{
  unsigned char bp_op; /* packet opcode type */
  unsigned char bp_htype; /* hardware addr type */
  unsigned char bp_hlen; /* hardware addr length */
  unsigned char bp_hops; /* gateway hops */
  unsigned char bp_xid[4]; /* transaction ID */
  uint16_t bp_secs; /* seconds since boot began */
  uint16_t bp_flags; /* RFC1532 broadcast, etc. */
  unsigned char bp_ciaddr[4]; /* client IP address */
  unsigned char bp_yiaddr[4]; /* 'your' IP address */
  unsigned char bp_siaddr[4]; /* (next) server IP address */
  unsigned char bp_riaddr[4]; /* relay IP address */
  unsigned char bp_chaddr[BP_CHADDR_LEN]; /* client hardware address */
  char bp_sname[BP_SNAME_LEN]; /* server host name */
  char bp_file[BP_FILE_LEN]; /* boot file name */

  // andrea: actually, I don't know what this is
  //unsigned char bp_vend[BP_VEND_LEN]; /* vendor-specific area */
  /* note that bp_vend can be longer, extending to end of packet. */

  unsigned char bp_mcookie[4]; /* andrea: Magic Cookie (?) */

} dhcp_packet;

typedef struct tlv_attribute
{
     /// Attribute number
     uint8_t  type;

     /// Total length, including header
     uint8_t  length;

     /// Value of the attribute. May have some internal structure, such as for VSAs etc
     uint8_t  value[RADIUS_MAX_ATTRIBUTE_SIZE];

} tlv_attribute_t;

#endif /* DHCP_H_ */

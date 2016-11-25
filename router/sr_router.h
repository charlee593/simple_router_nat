/*-----------------------------------------------------------------------------
 * File: sr_router.h
 * Date: ?
 * Authors: Guido Apenzeller, Martin Casado, Virkam V.
 * Contact: casado@stanford.edu
 *
 *---------------------------------------------------------------------------*/

#ifndef SR_ROUTER_H
#define SR_ROUTER_H

#include <netinet/in.h>
#include <sys/time.h>
#include <stdio.h>

#include "sr_protocol.h"
#include "sr_arpcache.h"
#include "sr_nat.h"

/* we dont like this debug , but what to do for varargs ? */
#ifdef _DEBUG_
#define Debug(x, args...) printf(x, ## args)
#define DebugMAC(x) \
  do { int ivyl; for(ivyl=0; ivyl<5; ivyl++) printf("%02x:", \
  (unsigned char)(x[ivyl])); printf("%02x",(unsigned char)(x[5])); } while (0)
#else
#define Debug(x, args...) do{}while(0)
#define DebugMAC(x) do{}while(0)
#endif

#define INIT_TTL 255
#define PACKET_DUMP_SIZE 1024

#define ECHO_REPLY       0
#define NET_UNREACHABLE  30
#define HOST_UNREACHABLE 31
#define PORT_UNREACHABLE 33
#define TIME_EXCEEDED    110

/* forward declare */
struct sr_if;
struct sr_rt;

/* ----------------------------------------------------------------------------
 * struct sr_instance
 *
 * Encapsulation of the state for a single virtual router.
 *
 * -------------------------------------------------------------------------- */

struct sr_instance
{
    int  sockfd;   /* socket to server */
    char user[32]; /* user name */
    char host[32]; /* host name */ 
    char template[30]; /* template name if any */
    unsigned short topo_id;
    struct sockaddr_in sr_addr; /* address to server */
    struct sr_if* if_list; /* list of interfaces */
    struct sr_rt* routing_table; /* routing table */
    struct sr_arpcache cache;   /* ARP cache */
    pthread_attr_t attr;
    FILE* logfile;
    struct sr_nat *nat;
};

/* -- sr_main.c -- */
int sr_verify_routing_table(struct sr_instance* sr);

/* -- sr_vns_comm.c -- */
int sr_send_packet(struct sr_instance* , uint8_t* , unsigned int , const char*);
int sr_connect_to_server(struct sr_instance* ,unsigned short , char* );
int sr_read_from_server(struct sr_instance* );

/* -- sr_router.c -- */
void sr_init(struct sr_instance* );
void sr_handlepacket(struct sr_instance* , uint8_t * , unsigned int , char* );
void reply_to_arp_req(struct sr_instance* sr, struct sr_ethernet_hdr* e_hdr, struct sr_arp_hdr* a_hdr, struct sr_if* iface);
void process_arp_reply(struct sr_instance* sr, struct sr_arp_hdr* arp_hdr, struct sr_if* iface);
void handle_ip_packet_for_router(struct sr_instance* sr, uint8_t* packet, unsigned int len, struct sr_ip_hdr* ip_hdr, struct sr_if* iface);
void handle_ip_packet_to_forward(struct sr_instance* sr, uint8_t* packet, unsigned int len, struct sr_ip_hdr* ip_hdr, struct sr_if* iface);
void send_echo_reply(struct sr_instance* sr, uint8_t* received_frame, char* from_interface, int len);
void send_icmp_t3_or_t11(struct sr_instance* sr, uint8_t* received_frame, char* from_interface, sr_icmp_type_t type, sr_icmp_dest_unreachable_code_t code);
struct sr_rt* lpm(struct sr_instance *sr, uint32_t target_ip);
int get_mask_len(uint32_t mask);

/* -- sr_if.c -- */
void sr_add_interface(struct sr_instance* , const char* );
void sr_set_ether_ip(struct sr_instance* , uint32_t );
void sr_set_ether_addr(struct sr_instance* , const unsigned char* );
void sr_print_if_list(struct sr_instance* );
#endif /* SR_ROUTER_H */


#ifndef SR_NAT_TABLE_H
#define SR_NAT_TABLE_H

#ifndef INTERNAL_INTERFACE
#define INTERNAL_INTERFACE "eth1"
#endif

#ifndef EXTERNAL_INTERFACE
#define EXTERNAL_INTERFACE "eth2"
#endif

#define STARTING_PORT_NUMBER  (1024)

#include <inttypes.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "sr_protocol.h"
#include "sr_router.h"
#include "sr_utils.h"
#include "sr_rt.h"

typedef enum {
  nat_mapping_icmp,
  nat_mapping_tcp
  /* nat_mapping_udp, */
} sr_nat_mapping_type;

typedef enum {
  closed,
  outbound_syn_sent,
  unsolicited_syn_received,
  syn_received,
  established,
  fin_1, /* Send FIN */
  fin_2, /* Receieved ACK for FIN */
  fin_3, /* Receieved FIN */
} tcp_conn_state;

struct sr_nat_connection {
	  /* add TCP connection state data members here */
	  uint32_t ip;
	  uint16_t port;
	  tcp_conn_state state;
	  uint8_t *unsolicited_packet;
	  time_t last_updated;

	  struct sr_nat_connection *next;
};

struct sr_nat_mapping {
  sr_nat_mapping_type type;
  uint32_t ip_int; /* internal ip addr */
  uint32_t ip_ext; /* external ip addr */
  uint16_t aux_int; /* internal port or icmp id */
  uint16_t aux_ext; /* external port or icmp id */
  time_t last_updated; /* use to timeout mappings */
  struct sr_nat_connection *conns; /* list of connections. null for ICMP */
  struct sr_nat_mapping *next;
};

struct sr_nat {
  /* add any fields here */
  struct sr_nat_mapping *mappings;

  int icmp_timeout;
  struct sr_instance *sr;

  int icmp_id;
  int tcp_port_num;


  /* threading */
  pthread_mutex_t lock;
  pthread_mutexattr_t attr;
  pthread_attr_t thread_attr;
  pthread_t thread;
};


int   sr_nat_init(struct sr_nat *nat);     /* Initializes the nat */
int   sr_nat_destroy(struct sr_nat *nat);  /* Destroys the nat (free memory) */
void *sr_nat_timeout(void *nat_ptr);  /* Periodic Timout */

/* Get the mapping associated with given external port.
   You must free the returned structure if it is not NULL. */
struct sr_nat_mapping *sr_nat_lookup_external(struct sr_nat *nat,
    uint16_t aux_ext, sr_nat_mapping_type type );

/* Get the mapping associated with given internal (ip, port) pair.
   You must free the returned structure if it is not NULL. */
struct sr_nat_mapping *sr_nat_lookup_internal(struct sr_nat *nat,
  uint32_t ip_int, uint16_t aux_int, sr_nat_mapping_type type );

/* Insert a new mapping into the nat's mapping table.
   You must free the returned structure if it is not NULL. */
struct sr_nat_mapping *sr_nat_insert_mapping(struct sr_nat *nat,
  uint32_t ip_int, uint16_t aux_int, sr_nat_mapping_type type );

void handle_nat_packet(struct sr_instance* sr, uint8_t * packet, unsigned int len, char* interface);
int get_port_num(struct sr_nat *nat, sr_nat_mapping_type type);
void nat_handle_icmp(struct sr_instance* sr, uint8_t * packet, unsigned int len, char* interface);
void nat_handle_tcp(struct sr_instance* sr, uint8_t * packet, unsigned int len, char* interface);

void nat_send_icmp_t3(struct sr_instance* sr, uint8_t* pkt, int len, uint32_t ip_sip);
#endif

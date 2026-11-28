/* vi: set sw=4 ts=4: */
/* Port to Busybox Copyright (C) 2006 Jesse Dutton <jessedutton@gmail.com>
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 *
 * DHCP Relay for 'DHCPv4 Configuration of IPSec Tunnel Mode' support
 * Copyright (C) 2002 Mario Strasser <mast@gmx.net>,
 *                   Zuercher Hochschule Winterthur,
 *                   Netbeat AG
 * Upstream has GPL v2 or later
 */

//usage:#define dhcprelay_trivial_usage
//usage:       "CLIENT_IFACE[,CLIENT_IFACE2]... SERVER_IFACE [SERVER_IP]"
//usage:#define dhcprelay_full_usage "\n\n"
//usage:       "Relay DHCP requests between clients and server"

#include "common.h"
#include "udhcp_relay_packet_bounds.h"
#include "udhcp_relay_xid_policy.h"
#include "udhcp_relay_select_fd_policy.h"
#include "udhcp_relay_bootrequest_policy.h"
#include "udhcp_relay_reply_policy.h"
#include "udhcp_relay_client_dest_policy.h"
#include "udhcp_relay_select_error_policy.h"

#define SERVER_PORT    67

/* lifetime of an xid entry in sec. */
#define MAX_LIFETIME   2*60
/* select timeout in sec. */
#define SELECT_TIMEOUT (MAX_LIFETIME / 8)

/* This list holds information about clients. The xid_* functions manipulate this list. */
struct xid_item {
	unsigned timestamp;
	int client;
	uint32_t xid;
	uint32_t gateway_nip;
	uint8_t htype;
	uint8_t hlen;
	uint8_t chaddr[16];
	struct sockaddr_in ip;
	struct xid_item *next;
} FIX_ALIASING;

#define dhcprelay_xid_list (*(struct xid_item*)&bb_common_bufsiz1)

static struct udhcp_relay_xid_budget xid_budget;

static struct xid_item *xid_add(uint32_t xid, const struct dhcp_packet *p,
				struct sockaddr_in *ip, int client, int *created)
{
	struct xid_item *item;

	/* DHCP clients retransmit with the same transaction ID and chaddr. */
	for (item = dhcprelay_xid_list.next; item != NULL; item = item->next) {
		if (item->xid == xid && item->client == client &&
		    item->gateway_nip == p->gateway_nip &&
		    item->htype == p->htype && item->hlen == p->hlen &&
		    memcmp(item->chaddr, p->chaddr, sizeof(item->chaddr)) == 0) {
			item->ip = *ip;
			item->timestamp = monotonic_sec();
			*created = 0;
			return item;
		}
	}

	/* Bound memory use when a client floods unique DHCP transactions. */
	if (!udhcp_relay_xid_reserve(&xid_budget))
		return NULL;

	item = xmalloc(sizeof(*item));
	item->ip = *ip;
	item->xid = xid;
	item->client = client;
	item->gateway_nip = p->gateway_nip;
	item->htype = p->htype;
	item->hlen = p->hlen;
	memcpy(item->chaddr, p->chaddr, sizeof(item->chaddr));
	item->timestamp = monotonic_sec();
	item->next = dhcprelay_xid_list.next;
	dhcprelay_xid_list.next = item;
	*created = 1;

	return item;
}

static void xid_expire(void)
{
	struct xid_item *item = dhcprelay_xid_list.next;
	struct xid_item *last = &dhcprelay_xid_list;
	unsigned current_time = monotonic_sec();

	while (item != NULL) {
		if ((current_time - item->timestamp) > MAX_LIFETIME) {
			last->next = item->next;
			udhcp_relay_xid_release(&xid_budget);
			free(item);
			item = last->next;
		} else {
			last = item;
			item = item->next;
		}
	}
}

static struct xid_item *xid_find(const struct dhcp_packet *p)
{
	struct xid_item *item = dhcprelay_xid_list.next;

	while (item != NULL) {
		if (item->xid == p->xid &&
		    item->gateway_nip == p->gateway_nip &&
		    item->htype == p->htype && item->hlen == p->hlen &&
		    memcmp(item->chaddr, p->chaddr, sizeof(item->chaddr)) == 0)
			return item;
		item = item->next;
	}
	return NULL;
}

/* Remove exactly one transaction; other clients may legitimately share xid. */
static void xid_del_item(struct xid_item *target)
{
	struct xid_item *item = dhcprelay_xid_list.next;
	struct xid_item *last = &dhcprelay_xid_list;

	while (item != NULL) {
		if (item == target) {
			last->next = item->next;
			udhcp_relay_xid_release(&xid_budget);
			free(item);
			return;
		}
		last = item;
		item = item->next;
	}
}

/**
 * get_dhcp_packet_type - gets the message type of a dhcp packet
 * p - pointer to the dhcp packet
 * returns the message type on success, -1 otherwise
 */
static int get_dhcp_packet_type(struct dhcp_packet *p)
{
	uint8_t *op = NULL;
	unsigned int option_length = 0;
	int option_status;

	/* it must be either a BOOTREQUEST or a BOOTREPLY */
	if (p->op != BOOTREQUEST && p->op != BOOTREPLY)
		return -1;

	/* DHCP_MESSAGE_TYPE is required and exactly one byte long. */
	option_status = udhcp_get_option_with_length(p, DHCP_MESSAGE_TYPE,
						     &op, &option_length);
	if (!udhcp_relay_message_type_length_valid(option_status,
						   option_length))
		return -1;

	return op[0];
}

/**
 * make_iface_list - parses client/server interface names
 * returns array
 */
static char **make_iface_list(char **client_and_server_ifaces, int *client_number)
{
	char *s, **iface_list;
	int i, cn;

	/* get number of items */
	cn = 2; /* 1 server iface + at least 1 client one */
	s = client_and_server_ifaces[0]; /* list of client ifaces */
	while (*s) {
		if (*s == ',')
			cn++;
		s++;
	}
	*client_number = cn;

	/* create vector of pointers */
	iface_list = xzalloc(cn * sizeof(iface_list[0]));

	iface_list[0] = client_and_server_ifaces[1]; /* server iface */

	i = 1;
	s = xstrdup(client_and_server_ifaces[0]); /* list of client ifaces */
	goto store_client_iface_name;

	while (i < cn) {
		if (*s++ == ',') {
			s[-1] = '\0';
 store_client_iface_name:
			iface_list[i++] = s;
		}
	}

	return iface_list;
}

/* Creates listen sockets (in fds) bound to client and server ifaces,
 * and returns numerically max fd.
 */
static int init_sockets(char **iface_list, int num_clients, int *fds)
{
	int i, j, n;

	n = 0;
	for (i = 0; i < num_clients; i++) {
		fds[i] = udhcp_listen_socket(/*INADDR_ANY,*/ SERVER_PORT, iface_list[i]);
		if (!udhcp_relay_fd_fits_select(fds[i], FD_SETSIZE)) {
			for (j = 0; j <= i; j++)
				close(fds[j]);
			bb_error_msg_and_die(
				"DHCP relay socket fd %d exceeds select fd_set capacity %d",
				fds[i], FD_SETSIZE);
		}
		if (n < fds[i])
			n = fds[i];
	}
	return n;
}

static int sendto_ip4(int sock, const void *msg, int msg_len, struct sockaddr_in *to)
{
	int err;

	errno = 0;
	err = sendto(sock, msg, msg_len, 0, (struct sockaddr*) to, sizeof(*to));
	err -= msg_len;
	if (err)
		bb_perror_msg("sendto");
	return err;
}

/**
 * pass_to_server() - forwards dhcp packets from client to server
 * p - packet to send
 * client - number of the client
 */
static void pass_to_server(struct dhcp_packet *p, int packet_len, int client, int *fds,
			struct sockaddr_in *client_addr, struct sockaddr_in *server_addr)
{
	struct xid_item *item;
	int type, created;

	/* check packet_type */
	type = get_dhcp_packet_type(p);
	if (type != DHCPDISCOVER && type != DHCPREQUEST
	 && type != DHCPDECLINE && type != DHCPRELEASE
	 && type != DHCPINFORM
	) {
		return;
	}

	/* DECLINE and RELEASE have no server reply to route to a client. */
	if (!udhcp_relay_xid_response_expected(type, DHCPDECLINE, DHCPRELEASE)) {
		(void)sendto_ip4(fds[0], p, packet_len, server_addr);
		return;
	}

	item = xid_add(p->xid, p, client_addr, client, &created);
	if (NULL == item) {
		log1("DHCP relay transaction table full; dropping request");
		return;
	}

	/* forward request to server */
	/* note that we send from fds[0] which is bound to SERVER_PORT (67).
	 * IOW: we send _from_ SERVER_PORT! Although this may look strange,
	 * RFC 1542 not only allows, but prescribes this for BOOTP relays.
	 */
	if (!udhcp_relay_send_result_ok(sendto_ip4(fds[0], p, packet_len, server_addr)) && created)
		xid_del_item(item);
}

/**
 * pass_to_client() - forwards dhcp packets from server to client
 * p - packet to send
 */
static void pass_to_client(struct dhcp_packet *p, int packet_len, int *fds)
{
	int type;
	struct xid_item *item;
	struct sockaddr_in client_dest;

	/* Reject other opcodes before parsing options or looking up a transaction. */
	if (!udhcp_relay_is_bootreply(p->op)) {
		log1("Ignoring non-BOOTREPLY packet on relay server socket");
		return;
	}

	type = get_dhcp_packet_type(p);
	if (type != DHCPOFFER && type != DHCPACK && type != DHCPNAK) {
		return;
	}

	/* Match the complete relay transaction, not just the random xid. */
	item = xid_find(p);
	if (NULL == item)
		return;

	client_dest = item->ip;
	client_dest.sin_port = htons(CLIENT_PORT);
	client_dest.sin_addr.s_addr = udhcp_relay_client_destination(
		item->ip.sin_addr.s_addr, p->ciaddr, p->yiaddr,
		(0 != (ntohs(p->flags) & 0x8000)), (DHCPNAK == type));
	

if (udhcp_relay_send_result_ok(sendto_ip4(fds[item->client], p, packet_len, &client_dest))) {
		xid_del_item(item);
	} else {
		log1("Unable to forward DHCP reply; retaining transaction for retry");
	}
}

int dhcprelay_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int dhcprelay_main(int argc, char **argv)
{
	struct sockaddr_in server_addr;
	char **iface_list;
	int *fds;
	int num_sockets, max_socket;
	uint32_t our_nip;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	server_addr.sin_port = htons(SERVER_PORT);

	/* dhcprelay CLIENT_IFACE1[,CLIENT_IFACE2...] SERVER_IFACE [SERVER_IP] */
	if (argc == 4) {
		if (!inet_aton(argv[3], &server_addr.sin_addr))
			bb_perror_msg_and_die("bad server IP");
	} else if (argc != 3) {
		bb_show_usage();
	}

	iface_list = make_iface_list(argv + 1, &num_sockets);

	fds = xmalloc(num_sockets * sizeof(fds[0]));

	/* Create sockets and bind one to every iface */
	max_socket = init_sockets(iface_list, num_sockets, fds);

	/* Get our IP on server_iface */
	if (udhcp_read_interface(argv[2], NULL, &our_nip, NULL, NULL))
		return 1;

	/* Main loop */
	while (1) {
// reinit stuff from time to time? go back to make_iface_list
// every N minutes?
		fd_set rfds;
		struct timeval tv;
		int i, select_result;

		FD_ZERO(&rfds);
		for (i = 0; i < num_sockets; i++)
			FD_SET(fds[i], &rfds);
		tv.tv_sec = SELECT_TIMEOUT;
		tv.tv_usec = 0;
		select_result = select(max_socket + 1, &rfds, NULL, NULL, &tv);
		if (select_result < 0) {
			int select_errno = errno;

			if (udhcp_relay_select_was_interrupted(select_errno))
				continue;
			errno = select_errno;
			bb_perror_msg_and_die("select");
		}
		if (select_result > 0) {
			int packlen;
			struct dhcp_packet dhcp_msg;

			/* server */
			if (FD_ISSET(fds[0], &rfds)) {
				/* read() cannot report that a datagram exceeded the buffer. */
				memset(&dhcp_msg, 0, sizeof(dhcp_msg));
				packlen = recvfrom(fds[0], &dhcp_msg, sizeof(dhcp_msg), MSG_TRUNC,
						NULL, NULL);
				if (udhcp_relay_packet_length_valid(packlen, sizeof(dhcp_msg),
						offsetof(struct dhcp_packet, options) + 4U) &&
				    dhcp_msg.cookie == htonl(DHCP_MAGIC)) {
					pass_to_client(&dhcp_msg, packlen, fds);
				} else if (packlen >= 0) {
					log1("Ignoring malformed or oversized DHCP server packet");
				}
			}

			/* clients */
			for (i = 1; i < num_sockets; i++) {
				struct sockaddr_in client_addr;
				socklen_t addr_size;

				if (!FD_ISSET(fds[i], &rfds))
					continue;

				/* recvfrom() otherwise leaves the tail of short datagrams
				 * as stale/uninitialized bytes for the option parser. */
				memset(&dhcp_msg, 0, sizeof(dhcp_msg));
				addr_size = sizeof(client_addr);
				packlen = recvfrom(fds[i], &dhcp_msg, sizeof(dhcp_msg), MSG_TRUNC,
						(struct sockaddr *)(&client_addr), &addr_size);
				if (!udhcp_relay_packet_length_valid(packlen, sizeof(dhcp_msg),
						offsetof(struct dhcp_packet, options) + 4U) ||
				    addr_size != sizeof(client_addr) ||
				    client_addr.sin_family != AF_INET ||
				    dhcp_msg.cookie != htonl(DHCP_MAGIC))
					continue;

				/* Get our IP on corresponding client_iface */
// RFC 1542
// 4.1 General BOOTP Processing for Relay Agents
// 4.1.1 BOOTREQUEST Messages
//   If the relay agent does decide to relay the request, it MUST examine
//   the 'giaddr' ("gateway" IP address) field.  If this field is zero,
//   the relay agent MUST fill this field with the IP address of the
//   interface on which the request was received.  If the interface has
//   more than one IP address logically associated with it, the relay
//   agent SHOULD choose one IP address associated with that interface and
//   use it consistently for all BOOTP messages it relays.  If the
//   'giaddr' field contains some non-zero value, the 'giaddr' field MUST
//   NOT be modified.  The relay agent MUST NOT, under any circumstances,
//   fill the 'giaddr' field with a broadcast address as is suggested in
//   [1] (Section 8, sixth paragraph).

// but why? what if server can't route such IP? Client ifaces may be, say, NATed!

// 4.1.2 BOOTREPLY Messages
//   BOOTP relay agents relay BOOTREPLY messages only to BOOTP clients.
//   It is the responsibility of BOOTP servers to send BOOTREPLY messages
//   directly to the relay agent identified in the 'giaddr' field.
// (yeah right, unless it is impossible... see comment above)
//   Therefore, a relay agent may assume that all BOOTREPLY messages it
//   receives are intended for BOOTP clients on its directly-connected
//   networks.
//
//   When a relay agent receives a BOOTREPLY message, it should examine
//   the BOOTP 'giaddr', 'yiaddr', 'chaddr', 'htype', and 'hlen' fields.
//   These fields should provide adequate information for the relay agent
//   to deliver the BOOTREPLY message to the client.
//
//   The 'giaddr' field can be used to identify the logical interface from
//   which the reply must be sent (i.e., the host or router interface
//   connected to the same network as the BOOTP client).  If the content
//   of the 'giaddr' field does not match one of the relay agent's
//   directly-connected logical interfaces, the BOOTREPLY messsage MUST be
//   silently discarded.
				if (!udhcp_relay_is_bootrequest(dhcp_msg.op)) {
					log1("Ignoring non-BOOTREQUEST packet on relay client socket");
					continue;
				}

				/* RFC 1542: discard received hops > 16 and increment otherwise. */
				if (!udhcp_relay_increment_hops(&dhcp_msg.hops)) {
					log1("DHCP relay hop count exceeded; dropping request");
					continue;
				}

				/* RFC 1542: a non-zero giaddr belongs to an upstream relay. */
				if (dhcp_msg.gateway_nip == 0) {
					uint32_t client_nip = 0;
					if (udhcp_read_interface(iface_list[i], NULL, &client_nip, NULL, NULL))
						client_nip = our_nip;
					dhcp_msg.gateway_nip = udhcp_relay_select_giaddr(
						dhcp_msg.gateway_nip, client_nip, our_nip);
				}
								pass_to_server(&dhcp_msg, packlen, i, fds, &client_addr, &server_addr);
			}
		}
		xid_expire();
	} /* while (1) */

	/* return 0; - not reached */
}

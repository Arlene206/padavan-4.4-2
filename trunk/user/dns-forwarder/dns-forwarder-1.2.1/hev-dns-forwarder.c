/*
 ============================================================================
 Name        : hev-dns-forwarder.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2014 everyone.
 Description : DNS Forwarder
 ============================================================================
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "hev-dns-forwarder.h"
#include "hev-dns-session.h"
#include "hev-dns-forwarder-policy.h"

#ifdef HEV_DNS_FORWARDER_TESTING
#include "../tests/hev_dns_forwarder_init_test_hooks.h"
#define DNS_FORWARDER_GETADDRINFO dns_forwarder_test_getaddrinfo
#define DNS_FORWARDER_FREEADDRINFO dns_forwarder_test_freeaddrinfo
#define DNS_FORWARDER_SOCKET dns_forwarder_test_socket
#define DNS_FORWARDER_IOCTL dns_forwarder_test_ioctl
#define DNS_FORWARDER_SETSOCKOPT dns_forwarder_test_setsockopt
#define DNS_FORWARDER_BIND dns_forwarder_test_bind
#define DNS_FORWARDER_CLOSE dns_forwarder_test_close
#define DNS_FORWARDER_EVENT_SOURCE_FDS_NEW dns_forwarder_test_event_source_fds_new
#define DNS_FORWARDER_EVENT_SOURCE_TIMEOUT_NEW dns_forwarder_test_event_source_timeout_new
#define DNS_FORWARDER_EVENT_SOURCE_SET_PRIORITY dns_forwarder_test_event_source_set_priority
#define DNS_FORWARDER_EVENT_SOURCE_ADD_FD dns_forwarder_test_event_source_add_fd
#define DNS_FORWARDER_EVENT_SOURCE_SET_CALLBACK dns_forwarder_test_event_source_set_callback
#define DNS_FORWARDER_EVENT_LOOP_ADD_SOURCE dns_forwarder_test_event_loop_add_source
#define DNS_FORWARDER_EVENT_LOOP_DEL_SOURCE dns_forwarder_test_event_loop_del_source
#define DNS_FORWARDER_EVENT_SOURCE_GET_LOOP dns_forwarder_test_event_source_get_loop
#define DNS_FORWARDER_EVENT_SOURCE_UNREF dns_forwarder_test_event_source_unref
#else
#define DNS_FORWARDER_GETADDRINFO getaddrinfo
#define DNS_FORWARDER_FREEADDRINFO freeaddrinfo
#define DNS_FORWARDER_SOCKET socket
#define DNS_FORWARDER_IOCTL ioctl
#define DNS_FORWARDER_SETSOCKOPT setsockopt
#define DNS_FORWARDER_BIND bind
#define DNS_FORWARDER_CLOSE close
#define DNS_FORWARDER_EVENT_SOURCE_FDS_NEW hev_event_source_fds_new
#define DNS_FORWARDER_EVENT_SOURCE_TIMEOUT_NEW hev_event_source_timeout_new
#define DNS_FORWARDER_EVENT_SOURCE_SET_PRIORITY hev_event_source_set_priority
#define DNS_FORWARDER_EVENT_SOURCE_ADD_FD hev_event_source_add_fd
#define DNS_FORWARDER_EVENT_SOURCE_SET_CALLBACK hev_event_source_set_callback
#define DNS_FORWARDER_EVENT_LOOP_ADD_SOURCE hev_event_loop_add_source
#define DNS_FORWARDER_EVENT_LOOP_DEL_SOURCE hev_event_loop_del_source
#define DNS_FORWARDER_EVENT_SOURCE_GET_LOOP hev_event_source_get_loop
#define DNS_FORWARDER_EVENT_SOURCE_UNREF hev_event_source_unref
#endif

#define TIMEOUT		(10 * 1000)

struct _HevDNSForwarder
{
	int listen_fd;
	unsigned int ref_count;
	HevEventSource *listener_source;
	HevEventSource *timeout_source;
	HevSList *session_list;

	HevEventLoop *loop;
	struct sockaddr_in upstream;
};

static bool listener_source_handler (HevEventSourceFD *fd, void *data);
static bool timeout_source_handler (void *data);
static void session_close_handler (HevDNSSession *session, void *data);
static void remove_all_sessions (HevDNSForwarder *self);
static int discard_pending_datagram (int fd);

HevDNSForwarder *
hev_dns_forwarder_new (HevEventLoop *loop, const char *addr, const char *port,
			const char *upstream, const char *upstream_port)
{
	HevDNSForwarder *self = NULL;
	struct in_addr upstream_addr;
	struct addrinfo hints;
	struct addrinfo *addr_ip = NULL;
	unsigned short parsed_upstream_port = 0;
	int nonblock = 1, reuseaddr = 1;
	int r = 0, port_status = 0, saved_errno = 0;

	/* Validate all caller-controlled inputs before acquiring resources. */
	if (!loop || !addr || !*addr || !port || !*port || !upstream) {
		errno = EINVAL;
		return NULL;
	}
	port_status = hev_dns_forwarder_parse_port (
		upstream_port, &parsed_upstream_port);
	if (0 != port_status) {
		errno = port_status;
		fprintf (stderr, "invalid upstream port %s\n",
			 upstream_port ? upstream_port : "(null)");
		return NULL;
	}
	if (0 == inet_aton (upstream, &upstream_addr)) {
		errno = EINVAL;
		fprintf (stderr, "invalid upstream %s\n", upstream);
		return NULL;
	}

	self = HEV_MEMORY_ALLOCATOR_ALLOC (sizeof (HevDNSForwarder));
	if (!self) {
		if (!errno)
			errno = ENOMEM;
		return NULL;
	}
	memset (self, 0, sizeof (*self));
	self->listen_fd = -1;
	self->ref_count = 1;
	self->session_list = NULL;
	self->loop = loop;
	self->upstream.sin_family = AF_INET;
	self->upstream.sin_addr = upstream_addr;
	self->upstream.sin_port = htons (parsed_upstream_port);

	/* Resolve the bind endpoint before opening the listener socket. */
	memset (&hints, 0, sizeof (hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	r = DNS_FORWARDER_GETADDRINFO (addr, port, &hints, &addr_ip);
	if (0 != r) {
		errno = EINVAL;
		goto error_exit;
	}

	self->listen_fd = DNS_FORWARDER_SOCKET (AF_INET, SOCK_DGRAM, 0);
	if (self->listen_fd < 0)
		goto error_exit;
	if (0 > DNS_FORWARDER_IOCTL (self->listen_fd, FIONBIO, (char *) &nonblock))
		goto error_exit;
	if (0 > DNS_FORWARDER_SETSOCKOPT (self->listen_fd, SOL_SOCKET, SO_REUSEADDR,
			    &reuseaddr, sizeof (reuseaddr)))
		goto error_exit;
	if (0 != DNS_FORWARDER_BIND (self->listen_fd, addr_ip->ai_addr, addr_ip->ai_addrlen))
		goto error_exit;
	DNS_FORWARDER_FREEADDRINFO (addr_ip);
	addr_ip = NULL;

	/* Every event-source allocation and registration is checked. */
	self->listener_source = DNS_FORWARDER_EVENT_SOURCE_FDS_NEW ();
	if (!self->listener_source)
		goto error_exit;
	DNS_FORWARDER_EVENT_SOURCE_SET_PRIORITY (self->listener_source, 1);
	if (!DNS_FORWARDER_EVENT_SOURCE_ADD_FD (
		self->listener_source, self->listen_fd, EPOLLIN | EPOLLET))
		goto error_exit;
	DNS_FORWARDER_EVENT_SOURCE_SET_CALLBACK (self->listener_source,
			(HevEventSourceFunc) listener_source_handler, self, NULL);
	if (!DNS_FORWARDER_EVENT_LOOP_ADD_SOURCE (loop, self->listener_source))
		goto error_exit;
	DNS_FORWARDER_EVENT_SOURCE_UNREF (self->listener_source);

	self->timeout_source = DNS_FORWARDER_EVENT_SOURCE_TIMEOUT_NEW (TIMEOUT);
	if (!self->timeout_source)
		goto error_exit;
	DNS_FORWARDER_EVENT_SOURCE_SET_PRIORITY (self->timeout_source, -1);
	DNS_FORWARDER_EVENT_SOURCE_SET_CALLBACK (self->timeout_source,
				 timeout_source_handler, self, NULL);
	if (!DNS_FORWARDER_EVENT_LOOP_ADD_SOURCE (loop, self->timeout_source))
		goto error_exit;
	DNS_FORWARDER_EVENT_SOURCE_UNREF (self->timeout_source);

	return self;

error_exit:
	saved_errno = errno ? errno : EIO;
	if (self->timeout_source) {
		if (DNS_FORWARDER_EVENT_SOURCE_GET_LOOP (self->timeout_source) == loop)
			DNS_FORWARDER_EVENT_LOOP_DEL_SOURCE (loop, self->timeout_source);
		else
			DNS_FORWARDER_EVENT_SOURCE_UNREF (self->timeout_source);
	}
	if (self->listener_source) {
		if (DNS_FORWARDER_EVENT_SOURCE_GET_LOOP (self->listener_source) == loop)
			DNS_FORWARDER_EVENT_LOOP_DEL_SOURCE (loop, self->listener_source);
		else
			DNS_FORWARDER_EVENT_SOURCE_UNREF (self->listener_source);
	}
	if (addr_ip)
		DNS_FORWARDER_FREEADDRINFO (addr_ip);
	if (self->listen_fd >= 0)
		DNS_FORWARDER_CLOSE (self->listen_fd);
	HEV_MEMORY_ALLOCATOR_FREE (self);
	errno = saved_errno;
	fprintf (stderr, "DNS Forwarder initialization failed: %s\n",
		 strerror (saved_errno));
	return NULL;
}

HevDNSForwarder *
hev_dns_forwarder_ref (HevDNSForwarder *self)
{
	if (self) {
		self->ref_count ++;
		return self;
	}

	return NULL;
}

void
hev_dns_forwarder_unref (HevDNSForwarder *self)
{
	if (self) {
		self->ref_count --;
		if (0 == self->ref_count) {
			DNS_FORWARDER_EVENT_LOOP_DEL_SOURCE (self->loop, self->listener_source);
			DNS_FORWARDER_EVENT_LOOP_DEL_SOURCE (self->loop, self->timeout_source);
			DNS_FORWARDER_CLOSE (self->listen_fd);
			remove_all_sessions (self);
			HEV_MEMORY_ALLOCATOR_FREE (self);
		}
	}
}

static int
discard_pending_datagram (int fd)
{
	char discard_byte;
	ssize_t size;

	do {
		/* A short receive still consumes one complete UDP datagram. */
		size = recvfrom (fd, &discard_byte, sizeof (discard_byte),
				 MSG_DONTWAIT, NULL, NULL);
	} while (size < 0 && EINTR == errno);

	return (size < 0) ? -1 : 0;
}

static bool
listener_source_handler (HevEventSourceFD *fd, void *data)
{
	HevDNSForwarder *self = data;

	for (;;) {
		HevDNSSession *session = NULL;
		HevEventSource *source = NULL;
		HevSList *new_sessions = NULL;
		ssize_t size;
		int action;

		/* EPOLLET requires draining every queued datagram to EAGAIN. */
		size = recvfrom (fd->fd, NULL, 0, MSG_PEEK | MSG_DONTWAIT,
				 NULL, NULL);
		action = hev_dns_forwarder_udp_drain_action (
				size, (size < 0) ? errno : 0);
		if (HEV_DNS_FORWARDER_UDP_DRAIN_RETRY == action)
			continue;
		if (HEV_DNS_FORWARDER_UDP_DRAIN_STOP == action) {
			fd->revents &= ~EPOLLIN;
			break;
		}
		if (HEV_DNS_FORWARDER_UDP_DRAIN_ERROR == action) {
			/* Avoid spinning on a persistent listener error. */
			fd->revents &= ~EPOLLIN;
			break;
		}

		session = hev_dns_session_new (
			fd->fd, &self->upstream, session_close_handler, self);
		if (!session) {
			if (discard_pending_datagram (fd->fd) < 0) {
				fd->revents &= ~EPOLLIN;
				break;
			}
			continue;
		}

		source = hev_dns_session_get_source (session);
		if (!source) {
			hev_dns_session_unref (session);
			if (discard_pending_datagram (fd->fd) < 0) {
				fd->revents &= ~EPOLLIN;
				break;
			}
			continue;
		}

		/*
		 * Track before activation: start() may synchronously close the
		 * session, and the close callback removes it from this list.
		 */
		new_sessions = hev_slist_append (self->session_list, session);
		if (!new_sessions) {
			hev_dns_session_unref (session);
			if (discard_pending_datagram (fd->fd) < 0) {
				fd->revents &= ~EPOLLIN;
				break;
			}
			continue;
		}
		self->session_list = new_sessions;

		if (!hev_event_loop_add_source (self->loop, source)) {
			self->session_list = hev_slist_remove (
				self->session_list, session);
			hev_dns_session_unref (session);
			if (discard_pending_datagram (fd->fd) < 0) {
				fd->revents &= ~EPOLLIN;
				break;
			}
			continue;
		}

		hev_dns_session_start (session);
	}

	return true;
}

static bool
timeout_source_handler (void *data)
{
	HevDNSForwarder *self = data;
	HevSList *list = NULL;
	for (list=self->session_list; list; list=hev_slist_next (list)) {
		HevDNSSession *session = hev_slist_data (list);
		if (hev_dns_session_get_idle (session)) {
			/* printf ("Remove timeout session %p\n", session); */
			hev_event_loop_del_source (self->loop,
						hev_dns_session_get_source (session));
			hev_dns_session_unref (session);
			hev_slist_set_data (list, NULL);
		} else {
			hev_dns_session_set_idle (session);
		}
	}
	self->session_list = hev_slist_remove_all (self->session_list, NULL);

	return true;
}

static void
session_close_handler (HevDNSSession *session, void *data)
{
	HevDNSForwarder *self = data;

	/* printf ("Remove session %p\n", session); */
	hev_event_loop_del_source (self->loop,
				hev_dns_session_get_source (session));
	hev_dns_session_unref (session);
	self->session_list = hev_slist_remove (self->session_list, session);
}

static void
remove_all_sessions (HevDNSForwarder *self)
{
	HevSList *list = NULL;
	for (list=self->session_list; list; list=hev_slist_next (list)) {
		HevDNSSession *session = hev_slist_data (list);
		/* printf ("Remove session %p\n", session); */
		hev_event_loop_del_source (self->loop,
					hev_dns_session_get_source (session));
		hev_dns_session_unref (session);
	}
	hev_slist_free (self->session_list);
}


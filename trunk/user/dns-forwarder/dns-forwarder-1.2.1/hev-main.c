/*
 ============================================================================
 Name        : hev-main.c
 Author      : Heiher <r@hev.cc>
 Copyright   : Copyright (c) 2014 everyone.
 Description : Main
 ============================================================================
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "hev-main.h"
#include "hev-dns-forwarder.h"
#include "hev-event-source-signal.h"

static const char *default_dns_servers = "8.8.8.8:53";
static const char *default_dns_port = "53";
static const char *default_listen_addr = "0.0.0.0";
static const char *default_listen_port = "5300";

static void
usage (const char *app)
{
	printf ("\
usage: %s [-h] [-b BIND_ADDR] [-p BIND_PORT] [-s DNS]\n\
Forwarding DNS queries on TCP transport.\n\
\n\
  -b BIND_ADDR          address that listens, default: 0.0.0.0\n\
  -p BIND_PORT          port that listens, default: 5300\n\
  -s DNS:[PORT]         DNS servers to use, default: 8.8.8.8:53\n\
  -h                    show this help message and exit\n", app);
}

static bool
signal_handler (void *data)
{
	HevEventLoop *loop = data;
	hev_event_loop_quit (loop);
	return false;
}

static bool
add_signal_source (HevEventLoop *loop, int signal_number)
{
	HevEventSource *source;

	source = hev_event_source_signal_new (signal_number);
	if (!source) {
		perror ("cannot create DNS Forwarder signal source");
		return false;
	}
	hev_event_source_set_priority (source, 3);
	hev_event_source_set_callback (source, signal_handler, loop, NULL);
	if (!hev_event_loop_add_source (loop, source)) {
		fprintf (stderr, "cannot register DNS Forwarder signal source\n");
		hev_event_source_unref (source);
		return false;
	}
	hev_event_source_unref (source);
	return true;
}

#include <stdlib.h>
int
main (int argc, char **argv)
{
	HevEventLoop *loop = NULL;
	HevDNSForwarder *forwarder = NULL;
	char *listen_addr = NULL;
	char *listen_port = NULL;
	char *dns_servers = NULL;
	char *dns_port = NULL;
	char *dns_delimiter = NULL;
	int dns_port_allocated = 0;
	int ch = 0, exit_status = 0;

	while ((ch = getopt (argc, argv, "hb:p:s:")) != -1) {
		switch (ch) {
			case 'h':
				usage (argv[0]);
				goto cleanup;
			case 'b': {
				char *replacement = strdup (optarg);
				if (!replacement)
					goto allocation_failed;
				free (listen_addr);
				listen_addr = replacement;
				break;
			}
			case 'p': {
				char *replacement = strdup (optarg);
				if (!replacement)
					goto allocation_failed;
				free (listen_port);
				listen_port = replacement;
				break;
			}
			case 's': {
				char *replacement = strdup (optarg);
				if (!replacement)
					goto allocation_failed;
				free (dns_servers);
				dns_servers = replacement;
				break;
			}
			default:
				exit_status = 2;
				goto cleanup;
		}
	}

	if (!dns_servers)
		dns_servers = strdup (default_dns_servers);
	if (!dns_servers)
		goto allocation_failed;

	dns_delimiter = strpbrk (dns_servers, ":#");
	if (dns_delimiter) {
		*dns_delimiter = '\0';
		dns_port = dns_delimiter + 1;
	} else {
		dns_port = strdup (default_dns_port);
		if (!dns_port)
			goto allocation_failed;
		dns_port_allocated = 1;
	}
	if (!listen_addr)
		listen_addr = strdup (default_listen_addr);
	if (!listen_port)
		listen_port = strdup (default_listen_port);
	if (!listen_addr || !listen_port)
		goto allocation_failed;

	loop = hev_event_loop_new ();
	if (!loop) {
		exit_status = 1;
		perror ("cannot create DNS Forwarder event loop");
		goto cleanup;
	}

	signal (SIGPIPE, SIG_IGN);
	/* Service managers send SIGTERM; interactive users commonly send SIGINT. */
	if (!add_signal_source (loop, SIGINT) ||
	    !add_signal_source (loop, SIGTERM)) {
		exit_status = 1;
		goto cleanup;
	}

	forwarder = hev_dns_forwarder_new (
		loop, listen_addr, listen_port, dns_servers, dns_port);
	if (!forwarder) {
		exit_status = 1;
		perror ("cannot initialize DNS Forwarder");
		goto cleanup;
	}

	hev_event_loop_run (loop);
	hev_dns_forwarder_unref (forwarder);
	forwarder = NULL;
	goto cleanup;

allocation_failed:
	if (!errno)
		errno = ENOMEM;
	perror ("DNS Forwarder argument allocation failed");
	exit_status = 1;

cleanup:
	if (forwarder)
		hev_dns_forwarder_unref (forwarder);
	if (loop)
		hev_event_loop_unref (loop);
	if (dns_port_allocated)
		free (dns_port);
	free (dns_servers);
	free (listen_addr);
	free (listen_port);
	return exit_status;
}

/*
   WSDD - Web Service Dynamic Discovery protocol server
  
   LLMNR responder

  	Copyright (c) 2016 NETGEAR
  	Copyright (c) 2016 Hiro Sugawara
  
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   This code is based on the wsdd implementation for Samba by
   community contributers.

   The original copyright comment follows.
 */

/*
   Unix SMB/CIFS implementation.

   Link-Local Multicast Name Resolution (LLMNR) helper functions
   (https://tools.ietf.org/html/rfc4795)

   Copyright (C) Tobias Waldvogel 2013
   Copyright (C) Jose M. Prieto 2015

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "wsdd.h"
#include "llmnr_question_policy.h"

/* set new debug class */
#undef  DBGC_CLASS
#define DBGC_CLASS wsdd_debug_level

#define DNS_TYPE_A	0x0001	/* rfc 1035 */
#define DNS_TYPE_AAAA	0x001C	/* rfc 3596 */
#define DNS_CLASS_IN	0x0001	/* rfc 1035 */

static void dumphex(const char *label, const void *p, size_t len)
{
	if (debug_L >= 3) {
		FILE *pp = popen("od -t x1c", "w");
		printf("%s", label);
		if (pp) {
			fwrite(p, len, 1, pp);
			pclose(pp);
		}
	}
}

static int llmnr_send_response(struct endpoint *ep,
				_saddr_t *sa,
				const uint8_t *in, size_t inlen,
				const char *myname)
{
	uint16_t qdcount, ancount, nscount, arcount;
	uint16_t qtype, qclass;
	char in_name[256];
	size_t question_end = 0, answer_len = 0, answer_pos = 0;
	size_t response_len = 0, response_capacity;
	uint8_t answer[28];
	uint8_t *out;
	uint8_t response_rcode = 0, extended_rcode = 0;
	struct llmnr_additional_info additional;
	int ret;
	_saddr_t ci;
	socklen_t slen = (sa->ss.ss_family == AF_INET)
				? sizeof sa->in
				: sizeof sa->in6;

	dumphex("LLMNR INPUT:\n", in, inlen);
	if (connected_if(sa, &ci)) {
		char buf[_ADDRSTRLEN];
		DEBUG(1, L, "llmnr: connected_if: %s",
			inet_ntop(sa->ss.ss_family, _SIN_ADDR(sa), buf, sizeof buf));
		return -1;
	}
	errno = EINVAL;
	if (inlen < 17) {
		DEBUG(1, L, "llmnr: packet shorter than DNS header and question");
		return -1;
	}
	if (in[2] & 0xF8) {
		DEBUG(1, L, "llmnr: not standard query");
		return -1;
	}
	if (in[2] & 0x04) {
		DEBUG(1, L, "llmnr: conflict bit set in query");
		return -1;
	}
	/* RFC 4795: a query's prohibited TC bit is ignored by responders. */
	qdcount = (uint16_t)(((uint16_t)in[4] << 8) | in[5]);
	if (qdcount != 1) {
		DEBUG(1, L, "llmnr: only one question entry is allowed, found %u",
			qdcount);
		return -1;
	}
	ancount = (uint16_t)(((uint16_t)in[6] << 8) | in[7]);
	nscount = (uint16_t)(((uint16_t)in[8] << 8) | in[9]);
	arcount = (uint16_t)(((uint16_t)in[10] << 8) | in[11]);
	if (ancount != 0 || nscount != 0) {
		DEBUG(1, L, "llmnr: answer and nameserver counts must be zero");
		return -1;
	}
	if (llmnr_parse_question_ex(in, inlen, in_name, sizeof(in_name),
				    &qtype, &qclass, &question_end) != 0) {
		DEBUG(1, L, "llmnr: malformed or truncated question section");
		return -1;
	}
	if (llmnr_parse_additional_section(in, inlen, question_end, arcount,
					   &additional) != 0) {
		DEBUG(1, L, "llmnr: malformed or truncated Additional section");
		return -1;
	}
	DEBUG(2, L, "llmnr: name in query %s", in_name);
	if (qtype != DNS_TYPE_A && qtype != DNS_TYPE_AAAA) {
		DEBUG(1, L, "llmnr: record in question is not A or AAAA");
		return -1;
	}
	if (qclass != DNS_CLASS_IN) {
		DEBUG(1, L, "llmnr: record is not class IN");
		return -1;
	}
	if (strlen(myname) != strlen(in_name) ||
	    strncasecmp(myname, in_name, strlen(in_name))) {
		DEBUG(2, L, "llmnr: not authoritative for name %s", in_name);
		return -1;
	}

	if (additional.duplicate_opt || additional.malformed_opt) {
		response_rcode = 1; /* FORMERR */
	} else if (additional.has_opt && additional.version != 0) {
		extended_rcode = 1; /* BADVERS */
	} else if (!((qtype == DNS_TYPE_A && sa->ss.ss_family == AF_INET6) ||
		     (qtype == DNS_TYPE_AAAA && sa->ss.ss_family == AF_INET))) {
		if (sa->ss.ss_family == AF_INET)
			answer_len = 12 + sizeof(ci.in.sin_addr);
		else if (sa->ss.ss_family == AF_INET6)
			answer_len = 12 + sizeof(ci.in6.sin6_addr);
		else {
			DEBUG(1, L, "%s: %d", strerror(EAFNOSUPPORT), sa->ss.ss_family);
			return -1;
		}
	}
	if (answer_len != 0) {
		answer[answer_pos++] = 0xC0;
		answer[answer_pos++] = 0x0C;
		answer[answer_pos++] = 0;
		answer[answer_pos++] = sa->ss.ss_family == AF_INET ? 1 : 28;
		answer[answer_pos++] = 0;
		answer[answer_pos++] = 1;
		answer[answer_pos++] = 0;
		answer[answer_pos++] = 0;
		answer[answer_pos++] = 0;
		answer[answer_pos++] = 0;
		answer[answer_pos++] = 0;
		if (sa->ss.ss_family == AF_INET) {
			answer[answer_pos++] = sizeof(ci.in.sin_addr);
			memcpy(answer + answer_pos, &ci.in.sin_addr, sizeof(ci.in.sin_addr));
			answer_pos += sizeof(ci.in.sin_addr);
		} else {
			answer[answer_pos++] = sizeof(ci.in6.sin6_addr);
			memcpy(answer + answer_pos, &ci.in6.sin6_addr, sizeof(ci.in6.sin6_addr));
			answer_pos += sizeof(ci.in6.sin6_addr);
		}
		answer_len = answer_pos;
	}

	response_capacity = question_end + answer_len +
			    (additional.has_opt ? 11U : 0U);
	out = calloc(response_capacity, 1);
	if (!out) {
		DEBUG(0, L, "llmnr: no memory for output buffer");
		return -1;
	}
	if (llmnr_build_response_packet(in, inlen, question_end,
					answer, answer_len, additional.has_opt,
					response_rcode, extended_rcode, out,
					response_capacity, &response_len) != 0) {
		free(out);
		return -1;
	}
	dumphex("LLMNR OUTPUT:\n", out, response_len);
	ret = sendto(ep->sock, out, response_len, 0, (struct sockaddr *)sa, slen);
	free(out);
	return ret;
}

int llmnr_init(struct endpoint *ep)
{
	return 0;
}

int llmnr_recv(struct endpoint *ep)
{
	uint8_t buf[10000];
	_saddr_t sa;
	socklen_t slen = sizeof sa;
	ssize_t len = recvfrom(ep->sock, buf, sizeof(buf)-1, 0,
				(struct sockaddr *)&sa, &slen);
	char name[HOST_NAME_MAX + 1];

	if (len > 0 && !llmnr_read_hostname(name, sizeof(name))) {
		buf[len] = '\0';
		llmnr_send_response(ep, &sa, buf, len, name);
	}

	return len;
}

int llmnr_timer(struct endpoint *ep)
{
	return 0;
}

void llmnr_exit(struct endpoint *ep)
{
}


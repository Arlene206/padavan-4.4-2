#ifndef MINIUPNPD_EVENT_CALLBACK_POLICY_H
#define MINIUPNPD_EVENT_CALLBACK_POLICY_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <string.h>

#define MINIUPNPD_EVENT_CALLBACK_HOST_MAX 48U

struct miniupnpd_event_callback {
	int family;
	unsigned short port;
	char host[MINIUPNPD_EVENT_CALLBACK_HOST_MAX];
	size_t path_offset;
};

static inline int
miniupnpd_event_callback_hex(unsigned char ch)
{
	return (ch >= '0' && ch <= '9') ||
	       (ch >= 'a' && ch <= 'f') ||
	       (ch >= 'A' && ch <= 'F');
}

/* Parse a bounded numeric-address callback URL with a mandatory URI path. */
static inline int
miniupnpd_event_parse_callback_url(const char *url, size_t url_len,
                                   struct miniupnpd_event_callback *parsed)
{
	size_t cursor = 7;
	size_t host_start, host_len, i;
	unsigned int port = 80;

	if (!url || !parsed || url_len < 9 ||
	    memcmp(url, "http://", 7) != 0)
		return -1;
	memset(parsed, 0, sizeof(*parsed));

	if (url[cursor] == '[') {
		struct in6_addr address;

		host_start = ++cursor;
		while (cursor < url_len && url[cursor] != ']')
			++cursor;
		if (cursor >= url_len || cursor == host_start)
			return -1;
		host_len = cursor - host_start;
		if (host_len >= sizeof(parsed->host))
			return -1;
		memcpy(parsed->host, url + host_start, host_len);
		parsed->host[host_len] = '\0';
		if (inet_pton(AF_INET6, parsed->host, &address) != 1)
			return -1;
		parsed->family = AF_INET6;
		++cursor;
	} else {
		struct in_addr address;

		host_start = cursor;
		while (cursor < url_len && url[cursor] != '/' &&
		       url[cursor] != ':')
			++cursor;
		host_len = cursor - host_start;
		if (host_len == 0 || host_len >= sizeof(parsed->host))
			return -1;
		memcpy(parsed->host, url + host_start, host_len);
		parsed->host[host_len] = '\0';
		if (inet_pton(AF_INET, parsed->host, &address) != 1)
			return -1;
		parsed->family = AF_INET;
	}

	if (cursor < url_len && url[cursor] == ':') {
		++cursor;
		if (cursor >= url_len || url[cursor] < '0' ||
		    url[cursor] > '9')
			return -1;
		port = 0;
		while (cursor < url_len && url[cursor] >= '0' &&
		       url[cursor] <= '9') {
			unsigned int digit = (unsigned int)(url[cursor] - '0');

			if (port > (65535U - digit) / 10U)
				return -1;
			port = port * 10U + digit;
			++cursor;
		}
		if (port == 0 || port > 65535U)
			return -1;
	}

	if (cursor >= url_len || url[cursor] != '/')
		return -1;
	for (i = cursor; i < url_len; ++i) {
		unsigned char ch = (unsigned char)url[i];

		if (ch <= 0x20U || ch == 0x7fU || ch == '<' ||
		    ch == '>' || ch == '#' || ch == '\\' || ch == '"')
			return -1;
		if (ch == '%' &&
		    (i + 2 >= url_len ||
		     !miniupnpd_event_callback_hex((unsigned char)url[i + 1]) ||
		     !miniupnpd_event_callback_hex((unsigned char)url[i + 2])))
			return -1;
	}
	parsed->port = (unsigned short)port;
	parsed->path_offset = cursor;
	return 0;
}

/* snprintf() returns the wanted length, which can exceed the allocated data. */
static inline int
miniupnpd_event_buffer_length_fits(int written, size_t capacity)
{
	return written >= 0 && (size_t)written < capacity;
}

#endif /* MINIUPNPD_EVENT_CALLBACK_POLICY_H */

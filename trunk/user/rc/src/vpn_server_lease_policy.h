#ifndef PADAVAN_VPN_SERVER_LEASE_POLICY_H
#define PADAVAN_VPN_SERVER_LEASE_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define VPN_SERVER_LEASE_IFNAME_SIZE 16
#define VPN_SERVER_LEASE_FIELD_SIZE 64
#define VPN_SERVER_LEASE_LINE_SIZE 256

/* Read one complete newline-terminated record. Reject binary, truncated and
 * overlong input rather than allowing a partial inventory to be published. */
static int
vpn_server_lease_read_line(FILE *fp, char *line, size_t line_size,
                           size_t *line_length)
{
	size_t used = 0;
	int ch;

	if (!fp || !line || line_size < 2 || !line_length) {
		errno = EINVAL;
		return -1;
	}

	while ((ch = fgetc(fp)) != EOF) {
		if (ch == '\0') {
			errno = EINVAL;
			return -1;
		}
		if (used + 1 >= line_size) {
			errno = EOVERFLOW;
			return -1;
		}
		line[used++] = (char)ch;
		if (ch == '\n') {
			line[used] = '\0';
			*line_length = used;
			return 1;
		}
	}

	if (ferror(fp)) {
		if (errno == 0)
			errno = EIO;
		return -1;
	}
	if (used != 0) {
		errno = EINVAL;
		return -1;
	}

	*line_length = 0;
	return 0;
}

/* Lease rows are written as: ifname local-address remote-address peer-name. */
static int
vpn_server_lease_parse_line(const char *line, size_t line_length,
                            char *ifname, size_t ifname_size)
{
	size_t pos = 0, token_start, token_length, peer_length;
	size_t field;

	if (!line || !ifname || ifname_size == 0 || line_length < 8 ||
	    line[line_length - 1] != '\n')
		return -1;

	for (pos = 0; pos + 1 < line_length; ++pos) {
		unsigned char ch = (unsigned char)line[pos];
		if (ch == '\0' || ch == '\r' || ch == '\t' || ch < 0x20 || ch == 0x7f)
			return -1;
	}

	pos = 0;
	for (field = 0; field < 3; ++field) {
		size_t limit = field == 0 ? ifname_size : VPN_SERVER_LEASE_FIELD_SIZE;

		token_start = pos;
		while (pos + 1 < line_length && line[pos] != ' ')
			++pos;
		token_length = pos - token_start;
		if (token_length == 0 || token_length >= limit || pos + 1 >= line_length)
			return -1;
		if (field == 0) {
			memcpy(ifname, line + token_start, token_length);
			ifname[token_length] = '\0';
		}
		while (pos + 1 < line_length && line[pos] == ' ')
			++pos;
	}

	peer_length = (line_length - 1) - pos;
	if (peer_length == 0 || peer_length >= VPN_SERVER_LEASE_FIELD_SIZE)
		return -1;

	return 0;
}

static int
vpn_server_lease_should_keep(const char *ifname, const char *disconnected_ifname)
{
	if (!ifname || !disconnected_ifname)
		return 0;
	return strcmp(ifname, disconnected_ifname) != 0;
}

#endif /* PADAVAN_VPN_SERVER_LEASE_POLICY_H */

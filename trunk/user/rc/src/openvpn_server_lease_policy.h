#ifndef PADAVAN_OPENVPN_SERVER_LEASE_POLICY_H
#define PADAVAN_OPENVPN_SERVER_LEASE_POLICY_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

static inline int
openvpn_server_lease_parse_line(const char *line, char ifname[16],
				char addr_l[64], char addr_r[64], char peer_name[64])
{
	int consumed = 0;
	int fields;

	if (!line || !ifname || !addr_l || !addr_r || !peer_name) {
		errno = EINVAL;
		return -1;
	}
	fields = sscanf(line, "%15s %63s %63s %63[^\n]%n",
			ifname, addr_l, addr_r, peer_name, &consumed);
	if (fields != 4 || (line[consumed] != '\0' && line[consumed] != '\n')) {
		errno = EINVAL;
		return -1;
	}
	return 0;
}

static inline int
openvpn_server_lease_line_matches(const char *line, const char *peer_addr_l,
				  const char *peer_addr_r)
{
	char ifname[16], addr_l[64], addr_r[64], peer_name[64];

	if (!peer_addr_l || !peer_addr_r ||
	    openvpn_server_lease_parse_line(line, ifname, addr_l, addr_r, peer_name) != 0) {
		errno = EINVAL;
		return -1;
	}
	return strcmp(addr_l, peer_addr_l) == 0 && strcmp(addr_r, peer_addr_r) == 0;
}

static inline int
openvpn_server_lease_copy(FILE *source, FILE *destination,
			  const char *peer_addr_l, const char *peer_addr_r,
			  int remove_match, int *trailing_newline)
{
	char line[256];

	if (!source || !destination || !trailing_newline ||
	    (remove_match && (!peer_addr_l || !peer_addr_r))) {
		errno = EINVAL;
		return -1;
	}
	*trailing_newline = 1;
	while (fgets(line, sizeof(line), source)) {
		char ifname[16], addr_l[64], addr_r[64], peer_name[64];
		int matches = 0;
		size_t length = strlen(line);

		if (!length || openvpn_server_lease_parse_line(line, ifname, addr_l,
							       addr_r, peer_name) != 0)
			return -1;
		if (remove_match)
			matches = strcmp(addr_l, peer_addr_l) == 0 &&
				  strcmp(addr_r, peer_addr_r) == 0;
		*trailing_newline = (line[length - 1] == '\n');
		if (!matches && fwrite(line, 1, length, destination) != length) {
			if (!errno)
				errno = EIO;
			return -1;
		}
	}
	if (ferror(source)) {
		errno = EIO;
		return -1;
	}
	return 0;
}

#endif /* PADAVAN_OPENVPN_SERVER_LEASE_POLICY_H */

#ifndef PADAVAN_OPENVPN_SERVER_LEASE_POLICY_H
#define PADAVAN_OPENVPN_SERVER_LEASE_POLICY_H

#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
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
	int ch;

	if (!source || !destination || !trailing_newline ||
	    (remove_match && (!peer_addr_l || !peer_addr_r))) {
		errno = EINVAL;
		return -1;
	}
	*trailing_newline = 1;
	for (;;) {
		char ifname[16], addr_l[64], addr_r[64], peer_name[64];
		size_t length = 0;
		int matches = 0, got_line = 0;

		while ((ch = fgetc(source)) != EOF) {
			got_line = 1;
			if (ch == '\0') {
				errno = EINVAL;
				return -1;
			}
			if (length + 1 >= sizeof(line)) {
				errno = EOVERFLOW;
				return -1;
			}
			line[length++] = (char)ch;
			if (ch == '\n')
				break;
		}
		if (ch == EOF && ferror(source)) {
			errno = EIO;
			return -1;
		}
		if (!got_line)
			break;
		line[length] = '\0';
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
		if (ch == EOF)
			break;
	}
	return 0;
}

/* client-connect/client-disconnect scripts are direct OpenVPN children. The
 * pidfile fence rejects a callback re-parented after stop, even if it acquires
 * the shared lease lock after the stop path has already cleared the snapshot. */
static inline int
openvpn_server_lease_pid_matches(const char *pid_text, size_t pid_length,
                                 pid_t parent_pid)
{
	size_t i;
	unsigned long value = 0;

	if (!pid_text || !pid_length || parent_pid <= (pid_t)1) {
		errno = EINVAL;
		return -1;
	}
	if (pid_text[pid_length - 1] == '\n')
		--pid_length;
	if (!pid_length) {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i < pid_length; ++i) {
		unsigned int digit;

		if (pid_text[i] < '0' || pid_text[i] > '9') {
			errno = EINVAL;
			return -1;
		}
		digit = (unsigned int)(pid_text[i] - '0');
		if (value > (ULONG_MAX - digit) / 10UL) {
			errno = ERANGE;
			return -1;
		}
		value = value * 10UL + digit;
	}
	if (value <= 1UL || (unsigned long)parent_pid != value) {
		errno = ESTALE;
		return 0;
	}
	return 1;
}

#endif /* PADAVAN_OPENVPN_SERVER_LEASE_POLICY_H */

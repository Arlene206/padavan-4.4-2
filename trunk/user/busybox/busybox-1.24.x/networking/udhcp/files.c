/* vi: set sw=4 ts=4: */
/*
 * DHCP server config and lease file manipulation
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */
#include <netinet/ether.h>

#include "common.h"
#include "dhcpd.h"
#include "udhcp_lease_file_atomic.h"

static int udhcp_open_lease_directory(const char *path)
{
	return open(path, O_RDONLY | O_DIRECTORY);
}
#include "udhcp_lease_file_time_policy.h"

/* on these functions, make sure your datatype matches */
static int FAST_FUNC read_str(const char *line, void *arg)
{
	char **dest = arg;

	free(*dest);
	*dest = xstrdup(line);
	return 1;
}

static int FAST_FUNC read_u32(const char *line, void *arg)
{
	*(uint32_t*)arg = bb_strtou32(line, NULL, 10);
	return errno == 0;
}

static int FAST_FUNC read_staticlease(const char *const_line, void *arg)
{
	char *line;
	char *mac_string;
	char *ip_string;
	struct ether_addr mac_bytes; /* it's "struct { uint8_t mac[6]; }" */
	uint32_t nip;

	/* Read mac */
	line = (char *) const_line;
	mac_string = strtok_r(line, " \t", &line);
	if (!mac_string || !ether_aton_r(mac_string, &mac_bytes))
		return 0;

	/* Read ip */
	ip_string = strtok_r(NULL, " \t", &line);
	if (!ip_string || !udhcp_str2nip(ip_string, &nip))
		return 0;

	add_static_lease(arg, (uint8_t*) &mac_bytes, nip);

	log_static_leases(arg);

	return 1;
}


struct config_keyword {
	const char *keyword;
	int (*handler)(const char *line, void *var) FAST_FUNC;
	void *var;
	const char *def;
};

static const struct config_keyword keywords[] = {
	/* keyword        handler           variable address               default */
	{"start"        , udhcp_str2nip   , &server_config.start_ip     , "192.168.0.20"},
	{"end"          , udhcp_str2nip   , &server_config.end_ip       , "192.168.0.254"},
	{"interface"    , read_str        , &server_config.interface    , "eth0"},
	/* Avoid "max_leases value not sane" warning by setting default
	 * to default_end_ip - default_start_ip + 1: */
	{"max_leases"   , read_u32        , &server_config.max_leases   , "235"},
	{"auto_time"    , read_u32        , &server_config.auto_time    , "7200"},
	{"decline_time" , read_u32        , &server_config.decline_time , "3600"},
	{"conflict_time", read_u32        , &server_config.conflict_time, "3600"},
	{"offer_time"   , read_u32        , &server_config.offer_time   , "60"},
	{"min_lease"    , read_u32        , &server_config.min_lease_sec, "60"},
	{"lease_file"   , read_str        , &server_config.lease_file   , LEASES_FILE},
	{"pidfile"      , read_str        , &server_config.pidfile      , "/var/run/udhcpd.pid"},
	{"siaddr"       , udhcp_str2nip   , &server_config.siaddr_nip   , "0.0.0.0"},
	/* keywords with no defaults must be last! */
	{"option"       , udhcp_str2optset, &server_config.options      , ""},
	{"opt"          , udhcp_str2optset, &server_config.options      , ""},
	{"notify_file"  , read_str        , &server_config.notify_file  , NULL},
	{"sname"        , read_str        , &server_config.sname        , NULL},
	{"boot_file"    , read_str        , &server_config.boot_file    , NULL},
	{"static_lease" , read_staticlease, &server_config.static_leases, ""},
};
enum { KWS_WITH_DEFAULTS = ARRAY_SIZE(keywords) - 6 };

void FAST_FUNC read_config(const char *file)
{
	parser_t *parser;
	const struct config_keyword *k;
	unsigned i;
	char *token[2];

	for (i = 0; i < KWS_WITH_DEFAULTS; i++)
		keywords[i].handler(keywords[i].def, keywords[i].var);

	parser = config_open(file);
	while (config_read(parser, token, 2, 2, "# \t", PARSE_NORMAL)) {
		for (k = keywords, i = 0; i < ARRAY_SIZE(keywords); k++, i++) {
			if (strcasecmp(token[0], k->keyword) == 0) {
				if (!k->handler(token[1], k->var)) {
					bb_error_msg("can't parse line %u in %s",
							parser->lineno, file);
					/* reset back to the default value */
					k->handler(k->def, k->var);
				}
				break;
			}
		}
	}
	config_close(parser);

	server_config.start_ip = ntohl(server_config.start_ip);
	server_config.end_ip = ntohl(server_config.end_ip);
}

void FAST_FUNC write_leases(void)
{
	static const struct udhcp_lease_file_io_ops file_io = {
		mkstemp, write, fsync, close, rename, unlink,
		udhcp_open_lease_directory, fsync, close
	};
	int write_status;
	unsigned i;
	size_t lease_count = 0;
	size_t total_size;
	leasetime_t curr;
	int64_t written_at;
	struct dyn_lease lease;
	unsigned char *buffer;
	unsigned char *cursor;

	for (i = 0; i < server_config.max_leases; i++) {
		if (g_leases[i].lease_nip != 0)
			lease_count++;
	}

	if (lease_count > ((size_t)-1 - sizeof(written_at)) / sizeof(lease)) {
		bb_error_msg("too many leases to serialize");
		return;
	}
	total_size = sizeof(written_at) + lease_count * sizeof(lease);
	buffer = malloc(total_size);
	if (!buffer) {
		bb_error_msg("cannot allocate lease-file buffer");
		return;
	}

	written_at = time(NULL);
	curr = written_at;
	written_at = SWAP_BE64(written_at);
	memcpy(buffer, &written_at, sizeof(written_at));
	cursor = buffer + sizeof(written_at);

	for (i = 0; i < server_config.max_leases; i++) {
		if (g_leases[i].lease_nip == 0)
			continue;

		memcpy(&lease, &g_leases[i], sizeof(lease));
		lease.expires -= curr;
		if ((signed_leasetime_t)lease.expires < 0)
			lease.expires = 0;
		lease.expires = htonl(lease.expires);
		memcpy(cursor, &lease, sizeof(lease));
		cursor += sizeof(lease);
	}

	write_status = udhcp_lease_file_write_atomic(server_config.lease_file,
			buffer, total_size, &file_io);
	if (write_status < 0) {
		bb_perror_msg("cannot atomically write leases to %s",
				server_config.lease_file);
		free(buffer);
		return;
	}
	if (write_status == UDHCP_LEASE_FILE_ATOMIC_RENAMED_UNSYNCED) {
		bb_perror_msg("lease file renamed but directory sync failed for %s",
				server_config.lease_file);
	}
	free(buffer);

	if (server_config.notify_file) {
		char *argv[3];
		argv[0] = server_config.notify_file;
		argv[1] = server_config.lease_file;
		argv[2] = NULL;
		spawn_and_wait(argv);
	}
}

void FAST_FUNC read_leases(const char *file)
{
	struct dyn_lease lease;
	int64_t written_at, time_passed;
	int fd;
#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
	unsigned i = 0;
#endif

	fd = open_or_warn(file, O_RDONLY);
	if (fd < 0)
		return;

	if (full_read(fd, &written_at, sizeof(written_at)) != sizeof(written_at))
		goto ret;
	written_at = SWAP_BE64(written_at);

	/* A bounded future timestamp can result from NTP clock correction.
	 * Treat it as zero elapsed time so valid leases are not discarded.
	 * Reject old-format files and timestamps outside the existing 12h window.
	 */
	if (!udhcp_lease_file_time_delta_valid(time(NULL), written_at, &time_passed))
		goto ret;

	while (full_read(fd, &lease, sizeof(lease)) == sizeof(lease)) {
		uint32_t y = ntohl(lease.lease_nip);
		if (y >= server_config.start_ip && y <= server_config.end_ip) {
			signed_leasetime_t expires = ntohl(lease.expires) - (signed_leasetime_t)time_passed;
			uint32_t static_nip;

			if (expires <= 0)
				continue;

			/* Check if there is a different static lease for this IP or MAC */
			static_nip = get_static_nip_by_mac(server_config.static_leases, lease.lease_mac);
			if (static_nip) {
				/* NB: we do not add lease even if static_nip == lease.lease_nip.
				 */
				continue;
			}
			if (is_nip_reserved(server_config.static_leases, lease.lease_nip))
				continue;

			/* NB: add_lease takes "relative time", IOW,
			 * lease duration, not lease deadline. */
			if (add_lease(lease.lease_mac, lease.lease_nip,
					expires,
					lease.hostname, sizeof(lease.hostname)
				) == 0
			) {
				bb_error_msg("too many leases while loading %s", file);
				break;
			}
#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
			i++;
#endif
		}
	}
	log1("Read %d leases", i);
 ret:
	close(fd);
}

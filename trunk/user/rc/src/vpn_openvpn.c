/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>

#include "rc.h"
#include "openvpn_client_secret_file.h"
#include "openvpn_config_file.h"
#include "openvpn_server_lease_policy.h"
#include "vpn_server_lease_lock.h"
#include <process_inventory.h>
#include "openvpn_process_stop_policy.h"
#include "vpn_server_restart_policy.h"
#include "route_status_policy.h"
#include "vpn_client_nvram_policy.h"
#include "vpn_server_config_policy.h"
#include "openvpn_ipv6_protocol_policy.h"
#include "openvpn_cert_input_policy.h"

#define OPENVPN_EXE		"/usr/sbin/openvpn"
#define COMMON_TEMP_DIR		"/tmp/openvpn"

#define SERVER_PID_FILE		VPN_SERVER_OPENVPN_PID_FILE
#define SERVER_ROOT_DIR		"/etc/openvpn/server"
#define SERVER_CERT_DIR		"/etc/storage/openvpn/server"
#define SERVER_LOG_NAME		"OpenVPN server"

#define CLIENT_PID_FILE		"/var/run/openvpn_cli.pid"
#define CLIENT_ROOT_DIR		"/etc/openvpn/client"
#define CLIENT_CERT_DIR		"/etc/storage/openvpn/client"
#define CLIENT_LOG_NAME		"OpenVPN client"

static const char *openvpn_server_keys[6] = {
	"ca.crt",
	"dh1024.pem",
	"server.crt",
	"server.key",
	"ta.key",
	"stc2.key"
};

static const char *openvpn_client_keys[4] = {
	"ca.crt",
	"client.crt",
	"client.key",
	"ta.key"
};

static int
openvpn_resolve_protocol_family(int *protocol, const char *operation)
{
	int ipv6_type = IPV6_DISABLED;
	int service_status, policy_status;

	if (!protocol || !operation) {
		errno = EINVAL;
		return EINVAL;
	}
	if (*protocol <= 1)
		return 0;

	service_status = get_ipv6_type_status(&ipv6_type);
	policy_status = openvpn_ipv6_protocol_apply_service_state(
		protocol, service_status, ipv6_type, IPV6_DISABLED);
	if (policy_status != 0) {
		errno = policy_status;
		logmessage("OpenVPN",
			   "cannot resolve IPv6 transport for %s (status=%d)",
			   operation, policy_status);
		return policy_status;
	}
	return 0;
}

static const char *env_ovpn[3] = {
	"dev",
	"ifconfig_local",
	"ifconfig_remote"
};

static const char *env_ovpn_alt[3] = {
	NULL,
	NULL,
	"route_vpn_gateway"
};

static const char *env_pppd[3] = {
	"IFNAME",
	"IPLOCAL",
	"IPREMOTE"
};

static const char *forbidden_list[] = {
	"dev ",
	"proto ",
	"topology ",
	"server ",
	"server-bridge ",
	"tmp-dir ",
	"writepid ",
	"auth ",
	"cipher ",
	"comp-lzo",
	"compress",
	"persist-key",
	"persist-tun",
	NULL
};

static int
openvpn_check_key(const char *key_name, int is_server)
{
	char key_file[64];

	snprintf(key_file, sizeof(key_file), "%s/%s", (is_server) ? SERVER_CERT_DIR : CLIENT_CERT_DIR, key_name);

	if (!check_if_file_exist(key_file)) {
		logmessage(LOGNAME, "Unable to start %s: key file \"%s\" not found!", 
			(is_server) ? SERVER_LOG_NAME : CLIENT_LOG_NAME, key_file);
		
		if (is_server)
			logmessage(SERVER_LOG_NAME, "Please manual build the certificates via \"%s\" script.", 
				"openvpn-cert.sh");
		return 0;
	}

	return 1;
}

static int
openvpn_create_client_secret(const char *secret_name)
{
	char secret_file[64];
	int path_len;

	path_len = snprintf(secret_file, sizeof(secret_file), "%s/%s",
			    CLIENT_ROOT_DIR, secret_name);
	if (path_len < 0 || (size_t)path_len >= sizeof(secret_file)) {
		errno = ENAMETOOLONG;
		return -1;
	}

	return openvpn_client_secret_publish(secret_file,
					     nvram_safe_get("vpnc_user"),
					     nvram_safe_get("vpnc_pass"));
}

static int
openvpn_safe_remote_host(const char *host)
{
	size_t i, len, label_len = 0;

	if (!host || !*host)
		return 0;
	if (is_valid_ipv4(host))
		return 1;
#if defined (USE_IPV6)
	if (is_valid_ipv6(host))
		return 1;
#endif

	len = strlen(host);
	if (len > 253)
		return 0;
	if (host[len - 1] == '.')
		len--;
	if (len == 0)
		return 0;

	for (i = 0; i < len; i++) {
		unsigned char c = (unsigned char)host[i];

		if (c == '.') {
			if (label_len == 0 || label_len > 63 || host[i - 1] == '-')
				return 0;
			label_len = 0;
		} else if ((c >= 'a' && c <= 'z') ||
			   (c >= 'A' && c <= 'Z') ||
			   (c >= '0' && c <= '9') || c == '-') {
			if (label_len == 0 && c == '-')
				return 0;
			if (++label_len > 63)
				return 0;
		} else {
			return 0;
		}
	}

	return (label_len > 0 && host[len - 1] != '-') ? 1 : 0;
}

static int
openvpn_safe_cipher_list(const char *value)
{
	const unsigned char *p = (const unsigned char *)value;
	size_t len;

	if (!p || !*p)
		return 0;
	len = strlen(value);
	if (len > 255)
		return 0;

	for (; *p; p++) {
		if (!((*p >= 'a' && *p <= 'z') ||
		      (*p >= 'A' && *p <= 'Z') ||
		      (*p >= '0' && *p <= '9') ||
		      *p == '-' || *p == '_' || *p == ':' || *p == ','))
			return 0;
	}

	return 1;
}

static int
openvpn_safe_path_component(const char *name)
{
	const unsigned char *p = (const unsigned char *)name;

	if (!p || !*p || strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return 0;

	for (; *p; p++) {
		if (*p < 32 || *p == 127 || *p == '/' || *p == '\\')
			return 0;
	}

	return 1;
}

struct openvpn_server_acl_entry {
	int present;
	unsigned int address_offset;
	char username[64];
	char remote_network[16];
	char remote_netmask[16];
};

struct openvpn_server_network_config {
	int is_tun;
	char lan_ip[INET_ADDRSTRLEN];
	char lan_netmask[INET_ADDRSTRLEN];
	uint32_t lan_address;
	uint32_t lan_mask;
	uint32_t virtual_network;
	uint32_t virtual_netmask;
	int tap_client_start;
	int tap_client_end;
};

struct openvpn_server_options_config {
	int tls_auth_mode;
	int tls_crypt_v2;
	int protocol;
	int redirect_gateway;
	int dhcp_enabled;
	int port;
	int auth_digest;
	int cipher;
	int compression;
	int wins_enabled;
	char ncp_cipher_list[257];
	char dns1[INET_ADDRSTRLEN];
	char dns2[INET_ADDRSTRLEN];
	char wins[INET_ADDRSTRLEN];
};

struct openvpn_client_config_snapshot {
	int is_tun;
	int auth_mode;
	int tls_mode;
	int protocol;
	int port;
	int auth_digest;
	int cipher;
	int compression;
	int redirect_gateway;
	int client_nat;
	char peer[257];
	char ncp_cipher_list[257];
};

static int
openvpn_server_transaction_get_int(struct nvram_transaction *transaction,
				   const char *key, int default_value,
				   int minimum, int maximum, int *result);


static int
openvpn_server_acl_copy_text(struct nvram_transaction *transaction,
			     const char *name, char *destination,
			     size_t destination_size)
{
	char *value = NULL;
	size_t length = 0;
	int read_status, read_errno;

	if (!transaction || !name || !destination || destination_size == 0) {
		errno = EINVAL;
		return -1;
	}
	errno = 0;
	read_status = nvram_transaction_get(transaction, name, &value);
	read_errno = errno;
	if (read_status < 0) {
		errno = read_errno > 0 ? read_errno : EIO;
		return -1;
	}
	if (read_status == 0) {
		destination[0] = '\0';
		return 0;
	}
	if (read_status != 1 || !value) {
		errno = EIO;
		return -1;
	}
	while (length < destination_size && value[length])
		++length;
	if (length >= destination_size) {
		errno = EOVERFLOW;
		return -1;
	}
	memcpy(destination, value, length);
	destination[length] = '\0';
	return 0;
}

static int
openvpn_server_acl_read_snapshot(struct openvpn_server_acl_entry *entries,
				 int *entry_count)
{
	struct nvram_transaction transaction;
	struct openvpn_server_acl_entry pending[MAX_CLIENTS_NUM];
	char key[32], *value = NULL;
	uint32_t parsed_address, parsed_mask;
	int count = 0, address_offset = 0, read_status, read_errno;
	int status, key_length, i;
	int result = -1, saved_errno = 0, end_status;

	if (!entries || !entry_count) {
		errno = EINVAL;
		return -1;
	}
	memset(pending, 0, sizeof(pending));
	*entry_count = 0;
	if (nvram_transaction_begin(&transaction) != 0)
		return -1;

	errno = 0;
	read_status = nvram_transaction_get(&transaction, "vpns_num_x", &value);
	read_errno = errno;
	status = vpn_server_config_int_status(read_status, read_errno, value,
					      0, 0, MAX_CLIENTS_NUM, &count);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}

	for (i = 0; i < count; ++i) {
		key_length = snprintf(key, sizeof(key), "vpns_user_x%d", i);
		if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
			errno = EOVERFLOW;
			goto finish_transaction;
		}
		if (openvpn_server_acl_copy_text(&transaction, key,
						 pending[i].username,
						 sizeof(pending[i].username)) != 0)
			goto finish_transaction;
		if (!pending[i].username[0])
			continue;
		if (!openvpn_safe_path_component(pending[i].username)) {
			errno = EINVAL;
			goto finish_transaction;
		}

		key_length = snprintf(key, sizeof(key), "vpns_rnet_x%d", i);
		if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
			errno = EOVERFLOW;
			goto finish_transaction;
		}
		if (openvpn_server_acl_copy_text(&transaction, key,
						 pending[i].remote_network,
						 sizeof(pending[i].remote_network)) != 0)
			goto finish_transaction;

		key_length = snprintf(key, sizeof(key), "vpns_rmsk_x%d", i);
		if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
			errno = EOVERFLOW;
			goto finish_transaction;
		}
		if (openvpn_server_acl_copy_text(&transaction, key,
						 pending[i].remote_netmask,
						 sizeof(pending[i].remote_netmask)) != 0)
			goto finish_transaction;

		/* Empty network and mask mean this account has no CCD route. */
		if (!pending[i].remote_network[0] && !pending[i].remote_netmask[0])
			continue;
		if (!pending[i].remote_network[0] || !pending[i].remote_netmask[0] ||
		    vpn_server_pool_parse_ipv4(pending[i].remote_network,
					       &parsed_address) != 0 ||
		    vpn_server_pool_parse_ipv4(pending[i].remote_netmask,
					       &parsed_mask) != 0 ||
		    vpn_server_route_netmask_status(parsed_mask) != 0) {
			errno = EINVAL;
			goto finish_transaction;
		}

		key_length = snprintf(key, sizeof(key), "vpns_addr_x%d", i);
		if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
			errno = EOVERFLOW;
			goto finish_transaction;
		}
		value = NULL;
		errno = 0;
		read_status = nvram_transaction_get(&transaction, key, &value);
		read_errno = errno;
		status = vpn_server_config_int_status(read_status, read_errno,
						      value, 0, 0, 255,
						      &address_offset);
		if (status != 0) {
			errno = status;
			goto finish_transaction;
		}
		pending[i].address_offset =
			(address_offset > 1 && address_offset < 255) ?
			(unsigned int)address_offset : 0U;
		pending[i].present = 1;
	}
	result = 0;

finish_transaction:
	if (result != 0)
		saved_errno = errno ? errno : EIO;
	end_status = nvram_transaction_end(&transaction);
	if (end_status != 0 && result == 0) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
	if (result != 0) {
		errno = saved_errno;
		return -1;
	}
	memcpy(entries, pending, sizeof(pending));
	*entry_count = count;
	return 0;
}

static int
openvpn_create_server_acl(FILE *fp, const char *ccd, unsigned int vnet, unsigned int vmsk)
{
	int i, entry_count, ccd_len;
	char vpns_ccd[64];
	struct openvpn_server_acl_entry entries[MAX_CLIENTS_NUM];
	struct stat ccd_stat;

	if (openvpn_server_acl_read_snapshot(entries, &entry_count) != 0) {
		int saved_errno = errno ? errno : EIO;

		logmessage(SERVER_LOG_NAME,
			   "cannot read complete OpenVPN server ACL snapshot (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return 0;
	}

	ccd_len = snprintf(vpns_ccd, sizeof(vpns_ccd), "%s/%s", SERVER_ROOT_DIR, ccd);
	if (ccd_len < 0 || ccd_len >= (int)sizeof(vpns_ccd)) {
		errno = ENAMETOOLONG;
		return 0;
	}
	if (mkdir(vpns_ccd, 0755) != 0 && errno != EEXIST) {
		logmessage(SERVER_LOG_NAME,
			   "cannot create client-config directory %s (errno=%d)",
			   vpns_ccd, errno);
		return 0;
	}
	if (lstat(vpns_ccd, &ccd_stat) != 0) {
		logmessage(SERVER_LOG_NAME,
			   "cannot inspect client-config directory %s (errno=%d)",
			   vpns_ccd, errno);
		return 0;
	}
	if (!S_ISDIR(ccd_stat.st_mode)) {
		errno = ENOTDIR;
		logmessage(SERVER_LOG_NAME,
			   "client-config path is not a directory: %s", vpns_ccd);
		return 0;
	}

	for (i = 0; i < entry_count; ++i) {
		FILE *fp_ccf;
		char ccf[80], staged_ccf[128];
		int ccf_len;
		unsigned int address_offset;

		if (!entries[i].present)
			continue;
		ccf_len = snprintf(ccf, sizeof(ccf), "%s/%s",
				   vpns_ccd, entries[i].username);
		if (ccf_len < 0 || ccf_len >= (int)sizeof(ccf)) {
			errno = ENAMETOOLONG;
			logmessage(SERVER_LOG_NAME,
				   "client-config path too long for user entry %d", i);
			return 0;
		}
		fp_ccf = openvpn_config_file_open(ccf, staged_ccf, sizeof(staged_ccf));
		if (!fp_ccf) {
			logmessage(SERVER_LOG_NAME,
				   "cannot stage client-config file %s (errno=%d)", ccf, errno);
			return 0;
		}
		address_offset = entries[i].address_offset;
		if (address_offset > 1U && address_offset < 255U) {
			struct in_addr pool_in;

			pool_in.s_addr = htonl((vnet & vmsk) | address_offset);
			fprintf(fp_ccf, "ifconfig-push %s %s\n",
				inet_ntoa(pool_in), VPN_SERVER_SUBNET_MASK);
			fprintf(fp, "route %s %s %s\n",
				entries[i].remote_network,
				entries[i].remote_netmask, inet_ntoa(pool_in));
		}
		fprintf(fp_ccf, "iroute %s %s\n",
			entries[i].remote_network, entries[i].remote_netmask);
		if (openvpn_config_file_publish(&fp_ccf, staged_ccf, ccf, 0644) != 0) {
			logmessage(SERVER_LOG_NAME,
				   "cannot publish client-config file %s (errno=%d)", ccf, errno);
			return 0;
		}
	}
	return 1;
}

static void
openvpn_add_auth(FILE *fp, int auth_idx)
{
	char *auth_str;

	switch (auth_idx)
	{
	case 0:
		auth_str = "MD5";
		break;
	case 1:
		auth_str = "SHA1";
		break;
	case 2:
		auth_str = "SHA224";
		break;
	case 3:
		auth_str = "SHA256";
		break;
	case 4:
		auth_str = "SHA384";
		break;
	case 5:
		auth_str = "SHA512";
		break;
	default:
		return;
	}

	fprintf(fp, "auth %s\n", auth_str);
}

static void
openvpn_add_cipher(FILE *fp, int cipher_idx, char *ncp_clist)
{
	char *cipher_str;

	switch (cipher_idx)
	{
	case 0:
		cipher_str = "none";
		break;
	case 1:
		cipher_str = "DES-CBC";
		break;
	case 2:
		cipher_str = "DES-EDE-CBC";
		break;
	case 3:
		cipher_str = "BF-CBC";
		break;
	case 4:
		cipher_str = "AES-128-CBC";
		break;
	case 5:
		cipher_str = "AES-192-CBC";
		break;
	case 6:
		cipher_str = "DES-EDE3-CBC";
		break;
	case 7:
		cipher_str = "DESX-CBC";
		break;
	case 8:
		cipher_str = "AES-256-CBC";
		break;
	case 9:
		cipher_str = "CAMELLIA-128-CBC";
		break;
	case 10:
		cipher_str = "CAMELLIA-192-CBC";
		break;
	case 11:
		cipher_str = "CAMELLIA-256-CBC";
		break;
	case 12:
		cipher_str = "AES-128-GCM";
		break;
	case 13:
		cipher_str = "AES-192-GCM";
		break;
	case 14:
		cipher_str = "AES-256-GCM";
		break;
	case 15:
		cipher_str = "CHACHA20-POLY1305";
		break;
	default:
		return;
	}

	fprintf(fp, "cipher %s\n", cipher_str);
	if (openvpn_safe_cipher_list(ncp_clist))
		fprintf(fp, "data-ciphers %s\n", ncp_clist);
}

static void
openvpn_add_compress(FILE *fp, int compress_idx, int is_server_mode)
{
	char *alg_str;

	switch (compress_idx)
	{
	case 1:
		/* also use for obtain compress from server */
		alg_str = "";
		break;
	case 2:
		alg_str = " lzo";
		break;
	case 3:
		alg_str = " lz4";
		break;
	case 4:
		alg_str = " lz4-v2";
		break;
	default:
		return;
	}

	fprintf(fp, "compress%s\n", alg_str);

	if (is_server_mode)
		fprintf(fp, "push \"compress%s\"\n", alg_str);
}

static int
openvpn_add_key(FILE *fp, const char *key_dir, const char *key_file, const char *key_sect)
{
	FILE *fpk = NULL;
	int fd = -1;
	int skip_head = 1, seen_end = 0;
	int saved_errno, path_len, secret_key;
	char buff[MAX_FILE_LINE_SIZE] = {0};
	struct stat st;

	if (!fp || !key_dir || !key_file || !key_sect) {
		errno = EINVAL;
		return -1;
	}
	path_len = snprintf(buff, sizeof(buff), "%s/%s", key_dir, key_file);
	if (path_len < 0 || (size_t)path_len >= sizeof(buff)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	fd = open(buff, O_RDONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC);
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() || st.st_nlink != 1) {
		errno = EPERM;
		goto failed;
	}
	secret_key = (strcmp(key_sect, "key") == 0 ||
		      strncmp(key_sect, "tls-", 4) == 0);
	if (secret_key && (st.st_mode & 0077) != 0) {
		errno = EPERM;
		goto failed;
	}
	fpk = fdopen(fd, "r");
	if (!fpk)
		goto failed;
	fd = -1;
	if (fprintf(fp, "<%s>\n", key_sect) < 0)
		goto failed;
	while (fgets(buff, sizeof(buff), fpk) != NULL) {
		if (skip_head) {
			if (!strstr(buff, "-----BEGIN "))
				continue;
			skip_head = 0;
		}
		if (fputs(buff, fp) == EOF)
			goto failed;
		if (strstr(buff, "-----END "))
			seen_end = 1;
	}
	if (ferror(fpk)) {
		if (!errno)
			errno = EIO;
		goto failed;
	}
	if (skip_head || !seen_end) {
		errno = EINVAL;
		goto failed;
	}
	if (fprintf(fp, "</%s>\n", key_sect) < 0)
		goto failed;
	if (fclose(fpk) != 0) {
		fpk = NULL;
		if (!errno)
			errno = EIO;
		goto failed;
	}
	fpk = NULL;
	return 0;
failed:
	saved_errno = errno ? errno : EIO;
	if (fpk)
		(void)fclose(fpk);
	else if (fd >= 0)
		(void)close(fd);
	errno = saved_errno;
	return -1;
}

static int
openvpn_export_remove_tempdir(const char *tmp_dir)
{
	int status, saved_errno;
	if (!tmp_dir || !tmp_dir[0]) {
		errno = EINVAL;
		return -1;
	}
	errno = 0;
	status = doSystem("rm -rf %s", tmp_dir);
	if (status != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(SERVER_LOG_NAME,
			   "failed to remove private OpenVPN export staging directory (status=%d, errno=%d)",
			   status, saved_errno);
		errno = saved_errno;
		return -1;
	}
	return 0;
}

static int
openvpn_server_transaction_get_int(struct nvram_transaction *transaction,
				   const char *key, int default_value,
				   int minimum, int maximum, int *result)
{
	char *value = NULL;
	int read_status, read_errno;

	if (!transaction || !key || !result) {
		errno = EINVAL;
		return -1;
	}
	errno = 0;
	read_status = nvram_transaction_get(transaction, key, &value);
	read_errno = errno;
	return vpn_server_config_int_status(read_status, read_errno, value,
					    default_value, minimum, maximum,
					    result);
}

static int
openvpn_read_server_network_snapshot(struct openvpn_server_network_config *network)
{
	struct nvram_transaction transaction;
	struct openvpn_server_network_config pending;
	char virtual_network_text[INET_ADDRSTRLEN];
	uint32_t lan_network, lan_broadcast, configured_virtual;
	uint32_t virtual_local, virtual_mask, virtual_start, virtual_end;
	int status, result = -1, saved_errno = 0, end_status;

	if (!network) {
		errno = EINVAL;
		return -1;
	}
	memset(&pending, 0, sizeof(pending));
	if (nvram_transaction_begin(&transaction) != 0)
		return -1;

	status = openvpn_server_transaction_get_int(&transaction,
						     "vpns_ov_mode", 0, 0, 1,
						     &pending.is_tun);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	if (openvpn_server_acl_copy_text(&transaction, "lan_ipaddr",
					 pending.lan_ip,
					 sizeof(pending.lan_ip)) != 0 ||
	    openvpn_server_acl_copy_text(&transaction, "lan_netmask",
					 pending.lan_netmask,
					 sizeof(pending.lan_netmask)) != 0)
		goto finish_transaction;
	if (vpn_server_pool_parse_ipv4(pending.lan_ip,
				       &pending.lan_address) != 0 ||
	    vpn_server_pool_parse_ipv4(pending.lan_netmask,
				       &pending.lan_mask) != 0) {
		errno = EINVAL;
		goto finish_transaction;
	}
	status = vpn_server_pool_validate_lan(pending.lan_address,
					      pending.lan_mask,
					      &lan_network, &lan_broadcast);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}

	if (pending.is_tun) {
		if (openvpn_server_acl_copy_text(&transaction, "vpns_vnet",
						 virtual_network_text,
						 sizeof(virtual_network_text)) != 0)
			goto finish_transaction;
		if (vpn_server_pool_parse_ipv4(virtual_network_text,
					       &configured_virtual) != 0) {
			errno = EINVAL;
			goto finish_transaction;
		}
		status = vpn_server_pool_resolve_virtual(configured_virtual,
							 pending.lan_address,
							 pending.lan_mask,
							 MAX_CLIENTS_NUM,
							 &virtual_local,
							 &virtual_mask,
							 &virtual_start,
							 &virtual_end);
		if (status != 0) {
			errno = status;
			goto finish_transaction;
		}
		pending.virtual_network = virtual_local & virtual_mask;
		pending.virtual_netmask = virtual_mask;
	} else {
		status = openvpn_server_transaction_get_int(&transaction,
							    "vpns_cli0", 245,
							    1, 254,
							    &pending.tap_client_start);
		if (status != 0) {
			errno = status;
			goto finish_transaction;
		}
		status = openvpn_server_transaction_get_int(&transaction,
							    "vpns_cli1", 254,
							    2, 254,
							    &pending.tap_client_end);
		if (status != 0) {
			errno = status;
			goto finish_transaction;
		}
	}
	result = 0;

finish_transaction:
	if (result != 0)
		saved_errno = errno ? errno : EIO;
	end_status = nvram_transaction_end(&transaction);
	if (end_status != 0 && result == 0) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
	if (result != 0) {
		errno = saved_errno;
		return -1;
	}
	*network = pending;
	return 0;
}

static int
openvpn_read_server_options_snapshot(struct openvpn_server_options_config *options)
{
	struct nvram_transaction transaction;
	struct openvpn_server_options_config pending;
	int status, result = -1, saved_errno = 0, end_status;

	if (!options) {
		errno = EINVAL;
		return -1;
	}
	memset(&pending, 0, sizeof(pending));

	/* This checked helper reads dhcp_enable_x without nesting transaction locks. */
	status = is_dhcpd_enabled_status(0, &pending.dhcp_enabled);
	if (status != 0) {
		errno = status;
		return -1;
	}
	if (pending.dhcp_enabled != 0 && pending.dhcp_enabled != 1) {
		errno = EINVAL;
		return -1;
	}

	if (nvram_transaction_begin(&transaction) != 0)
		return -1;
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpns_ov_atls", 0, 0, 2,
						    &pending.tls_auth_mode);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpns_ov_tcv2", 0, 0, 1,
						    &pending.tls_crypt_v2);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpns_ov_prot", 0, 0, 5,
						    &pending.protocol);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpns_ov_rdgw", 0, 0, 1,
						    &pending.redirect_gateway);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpns_ov_port", 1194, 1, 65535,
						    &pending.port);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpns_ov_mdig", 0, 0, 5,
						    &pending.auth_digest);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpns_ov_ciph", 0, 0, 15,
						    &pending.cipher);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpns_ov_compress", 0, 0, 4,
						    &pending.compression);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "wins_enable", 0, 0, 1,
						    &pending.wins_enabled);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}

	if (openvpn_server_acl_copy_text(&transaction, "vpns_ov_ncp_clist",
					 pending.ncp_cipher_list,
					 sizeof(pending.ncp_cipher_list)) != 0 ||
	    openvpn_server_acl_copy_text(&transaction, "dhcp_dns1_x",
					 pending.dns1, sizeof(pending.dns1)) != 0 ||
	    openvpn_server_acl_copy_text(&transaction, "dhcp_dns2_x",
					 pending.dns2, sizeof(pending.dns2)) != 0 ||
	    openvpn_server_acl_copy_text(&transaction, "dhcp_wins_x",
					 pending.wins, sizeof(pending.wins)) != 0)
		goto finish_transaction;

	result = 0;

finish_transaction:
	if (result != 0)
		saved_errno = errno ? errno : EIO;
	end_status = nvram_transaction_end(&transaction);
	if (end_status != 0 && result == 0) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
	if (result != 0) {
		errno = saved_errno;
		return -1;
	}
	*options = pending;
	return 0;
}

static int
openvpn_create_server_conf(const char *conf_file,
				 const struct openvpn_server_network_config *network,
				 const struct openvpn_server_options_config *options)
{
	FILE *fp;
	char staged_conf[128];
	int i, i_prot, i_prot_ori, i_atls, i_tcv2, i_rdgw, i_dhcp, i_items;
	int protocol_status;
	unsigned int laddr, lmask;
	const char *lanip, *lannm;
	char *wins, *dns1, *dns2;
	const char *p_prot;
	struct in_addr pool_in;

	if (!network || !options) {
		errno = EINVAL;
		return 1;
	}
	i_atls = options->tls_auth_mode;
	i_tcv2 = options->tls_crypt_v2;

	for (i=0; i<6; i++) {
		if (!i_atls && (i == 4))
			continue;
		if (!i_tcv2 && (i == 5))
			continue;
		if (!openvpn_check_key(openvpn_server_keys[i], 1))
			return 1;
	}

	lanip = network->lan_ip;
	lannm = network->lan_netmask;
	laddr = network->lan_address;
	lmask = network->lan_mask;


	i_prot = options->protocol;
	i_rdgw = options->redirect_gateway;
	i_dhcp = options->dhcp_enabled;

	i_prot_ori = i_prot;
	protocol_status = openvpn_resolve_protocol_family(
		&i_prot, "server configuration");
	if (protocol_status != 0)
		return 1;

	/* note: upcoming openvpn 2.4 will need direct set udp4/tcp4-server for ipv4 only */
#if defined (USE_IPV6)
	if (i_prot == 3)
		p_prot = "tcp6-server";
	else if (i_prot == 2)
		p_prot = "udp6";
	else if (i_prot == 5)
		p_prot = "tcp-server";
	else if (i_prot == 4)
		p_prot = "udp";
	else
#endif
	if (i_prot == 1)
		p_prot = "tcp4-server";
	else
		p_prot = "udp4";

	/* Keep the configured protocol unchanged; use the supported IPv4 fallback only here. */

	fp = openvpn_config_file_open(conf_file, staged_conf,
				       sizeof(staged_conf));
	if (!fp)
		return 1;

	fprintf(fp, "proto %s\n", p_prot);
	fprintf(fp, "port %d\n", options->port);

	if (network->is_tun) {
		unsigned int vnet, vmsk;

		vnet = network->virtual_network;
		vmsk = network->virtual_netmask;
		pool_in.s_addr = htonl(vnet & vmsk);
		
		fprintf(fp, "dev %s\n", IFNAME_SERVER_TUN);
		fprintf(fp, "topology %s\n", "subnet");
		fprintf(fp, "server %s %s\n", inet_ntoa(pool_in), VPN_SERVER_SUBNET_MASK);
		fprintf(fp, "client-config-dir %s\n", "ccd");
		
		if (!openvpn_create_server_acl(fp, "ccd", vnet, vmsk)) {
			int saved_errno = errno ? errno : EIO;

			if (fclose(fp) != 0)
				logmessage(SERVER_LOG_NAME, "failed to close staged server config after CCD error");
			fp = NULL;
			if (unlink(staged_conf) != 0 && errno != ENOENT)
				logmessage(SERVER_LOG_NAME, "failed to remove staged server config after CCD error");
			errno = saved_errno;
			return 1;
		}
		
		pool_in.s_addr = htonl(laddr & lmask);
		fprintf(fp, "push \"route %s %s\"\n", inet_ntoa(pool_in), lannm);
	} else {
		char sp_b[INET_ADDRSTRLEN], sp_e[INET_ADDRSTRLEN];
		unsigned int vp_b, vp_e, lnet;
		
		lnet = ~(lmask) - 1;
		vp_b = (unsigned int)network->tap_client_start;
		vp_e = (unsigned int)network->tap_client_end;
		if (vp_b > lnet)
			vp_b = lnet;
		if (vp_e > lnet)
			vp_e = lnet;
		if (vp_e < vp_b)
			vp_e = vp_b;
		
		pool_in.s_addr = htonl((laddr & lmask) | vp_b);
		strcpy(sp_b, inet_ntoa(pool_in));
		
		pool_in.s_addr = htonl((laddr & lmask) | vp_e);
		strcpy(sp_e, inet_ntoa(pool_in));
		
		fprintf(fp, "dev %s\n", IFNAME_SERVER_TAP);
		fprintf(fp, "server-bridge %s %s %s %s\n", lanip, lannm, sp_b, sp_e);
	}

	openvpn_add_auth(fp, options->auth_digest);
	openvpn_add_cipher(fp, options->cipher, options->ncp_cipher_list);
	openvpn_add_compress(fp, options->compression, 1);

	i_items = 0;
	if (i_rdgw) {
		fprintf(fp, "push \"redirect-gateway def1 %s\"\n", "bypass-dhcp");
		
		if (i_dhcp) {
			dns1 = (char *)options->dns1;
			dns2 = (char *)options->dns2;
			if (is_valid_ipv4(dns1)) {
				i_items++;
				fprintf(fp, "push \"dhcp-option %s %s\"\n", "DNS", dns1);
			}
			if (is_valid_ipv4(dns2) && strcmp(dns2, dns1)) {
				i_items++;
				fprintf(fp, "push \"dhcp-option %s %s\"\n", "DNS", dns2);
			}
		}
		
		if (i_items < 1)
			fprintf(fp, "push \"dhcp-option %s %s\"\n", "DNS", lanip);
	}

	i_items = 0;
	if (i_dhcp) {
		wins = (char *)options->wins;
		if (is_valid_ipv4(wins)) {
			i_items++;
			fprintf(fp, "push \"dhcp-option %s %s\"\n", "WINS", wins);
		}
	}

#if defined(APP_SMBD) || defined(APP_NMBD)
	if ((i_items < 1) && options->wins_enabled)
		fprintf(fp, "push \"dhcp-option %s %s\"\n", "WINS", lanip);
#endif

	fprintf(fp, "ca %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[0]);
	fprintf(fp, "dh %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[1]);
	fprintf(fp, "cert %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[2]);
	fprintf(fp, "key %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[3]);

	if (i_atls == 1) {
		fprintf(fp, "tls-auth %s/%s %d\n", SERVER_CERT_DIR, openvpn_server_keys[4], 0);
	} else if (i_atls == 2) {
		fprintf(fp, "tls-crypt %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[4]);
	}

	if (i_tcv2) {
		fprintf(fp, "tls-crypt-v2 %s/%s\n", SERVER_CERT_DIR, openvpn_server_keys[5]);
	}

	fprintf(fp, "persist-key\n");
	fprintf(fp, "persist-tun\n");
	fprintf(fp, "user %s\n", SYS_USER_NOBODY);
	fprintf(fp, "group %s\n", SYS_GROUP_NOGROUP);
	fprintf(fp, "script-security %d\n", 2);
	fprintf(fp, "tmp-dir %s\n", COMMON_TEMP_DIR);
	fprintf(fp, "writepid %s\n", SERVER_PID_FILE);

	fprintf(fp, "client-connect %s\n", SCRIPT_OVPN_SERVER);
	fprintf(fp, "client-disconnect %s\n", SCRIPT_OVPN_SERVER);

	fprintf(fp, "\n### User params:\n");

	load_user_config(fp, SERVER_CERT_DIR, "server.conf", forbidden_list);

	if (openvpn_config_file_publish(&fp, staged_conf, conf_file, 0644) != 0) {
		int saved_errno = errno ? errno : EIO;
		logmessage(SERVER_LOG_NAME,
			   "cannot publish OpenVPN server configuration (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return 1;
	}

	return 0;
}

static int
openvpn_read_client_config_snapshot(struct openvpn_client_config_snapshot *config)
{
	struct nvram_transaction transaction;
	struct openvpn_client_config_snapshot pending;
	int status, result = -1, saved_errno = 0, end_status;

	if (!config) {
		errno = EINVAL;
		return -1;
	}
	memset(&pending, 0, sizeof(pending));
	if (nvram_transaction_begin(&transaction) != 0)
		return -1;

	status = openvpn_server_transaction_get_int(&transaction,
						    "vpnc_ov_mode", 0, 0, 1,
						    &pending.is_tun);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpnc_ov_auth", 0, 0, 1,
						    &pending.auth_mode);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpnc_ov_atls", 0, 0, 3,
						    &pending.tls_mode);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpnc_ov_prot", 0, 0, 5,
						    &pending.protocol);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpnc_ov_port", 1194, 1, 65535,
						    &pending.port);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpnc_ov_mdig", 0, 0, 5,
						    &pending.auth_digest);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpnc_ov_ciph", 0, 0, 15,
						    &pending.cipher);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpnc_ov_compress", 0, 0, 4,
						    &pending.compression);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpnc_dgw", 0, 0, 1,
						    &pending.redirect_gateway);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	status = openvpn_server_transaction_get_int(&transaction,
						    "vpnc_ov_cnat", 0, 0, 1,
						    &pending.client_nat);
	if (status != 0) {
		errno = status;
		goto finish_transaction;
	}
	if (openvpn_server_acl_copy_text(&transaction, "vpnc_peer",
					 pending.peer, sizeof(pending.peer)) != 0 ||
	    openvpn_server_acl_copy_text(&transaction, "vpnc_ov_ncp_clist",
					 pending.ncp_cipher_list,
					 sizeof(pending.ncp_cipher_list)) != 0)
		goto finish_transaction;
	if (!openvpn_safe_remote_host(pending.peer)) {
		errno = EINVAL;
		goto finish_transaction;
	}
	result = 0;

finish_transaction:
	if (result != 0)
		saved_errno = errno ? errno : EIO;
	end_status = nvram_transaction_end(&transaction);
	if (end_status != 0 && result == 0) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
	if (result != 0) {
		errno = saved_errno;
		return -1;
	}
	*config = pending;
	return 0;
}

static int
openvpn_create_client_conf(const char *conf_file,
			   const struct openvpn_client_config_snapshot *config)
{
	FILE *fp;
	char staged_conf[128];
	int i, i_prot, i_prot_ori, i_auth, i_atls;
	int protocol_status;
	const char *p_peer, *p_prot;

	if (!config) {
		errno = EINVAL;
		return 1;
	}
	i_auth = config->auth_mode;
	i_atls = config->tls_mode;

	for (i=0; i<4; i++) {
		if (i_auth == 1 && (i == 1 || i == 2))
			continue;
		if (!i_atls && (i == 3))
			continue;
		if (!openvpn_check_key(openvpn_client_keys[i], 0))
			return 1;
	}

	i_prot = config->protocol;
	i_prot_ori = i_prot;
	protocol_status = openvpn_resolve_protocol_family(
		&i_prot, "client configuration");
	if (protocol_status != 0)
		return 1;

	p_peer = config->peer;

	/* note: upcoming openvpn 2.4 will need direct set udp4/tcp4-client for ipv4 only */
#if defined (USE_IPV6)
	/* check peer address is direct ipv4/ipv6 */
	if (i_prot > 1 && is_valid_ipv4(p_peer))
		i_prot &= 1;
	else
	if (i_prot < 2 && is_valid_ipv6(p_peer))
		i_prot += 2;

	if (i_prot == 3)
		p_prot = "tcp6-client";
	else if (i_prot == 2)
		p_prot = "udp6";
	else if (i_prot == 5)
		p_prot = "tcp-client";
	else if (i_prot == 4)
		p_prot = "udp";
	else
#endif
	if (i_prot == 1)
		p_prot = "tcp4-client";
	else
		p_prot = "udp4";

	/* The detected peer family affects this generated config only. */

	/* Publish auth credentials privately before writing a config that uses them. */
	if (i_auth == 1 && openvpn_create_client_secret("secret") != 0) {
		logmessage(CLIENT_LOG_NAME,
			   "Unable to publish VPN client credentials (errno=%d).",
			   errno ? errno : EIO);
		return 1;
	}

	fp = openvpn_config_file_open(conf_file, staged_conf,
				       sizeof(staged_conf));
	if (!fp)
		return 1;

	fprintf(fp, "client\n");
	fprintf(fp, "proto %s\n", p_prot);
	fprintf(fp, "remote %s %d\n", p_peer, config->port);
	fprintf(fp, "resolv-retry %s\n", "infinite");
	fprintf(fp, "nobind\n");

	fprintf(fp, "dev %s\n", (config->is_tun) ? IFNAME_CLIENT_TUN : IFNAME_CLIENT_TAP);

	fprintf(fp, "ca %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[0]);
	if (i_auth == 0) {
		fprintf(fp, "cert %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[1]);
		fprintf(fp, "key %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[2]);
	}

	if (i_atls == 1) {
		fprintf(fp, "tls-auth %s/%s %d\n", CLIENT_CERT_DIR, openvpn_client_keys[3], 1);
	} else if (i_atls == 2) {
		fprintf(fp, "tls-crypt %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[3]);
	} else if (i_atls == 3) {
		fprintf(fp, "tls-crypt-v2 %s/%s\n", CLIENT_CERT_DIR, openvpn_client_keys[3]);
	}

	openvpn_add_auth(fp, config->auth_digest);
	openvpn_add_cipher(fp, config->cipher, config->ncp_cipher_list);
	openvpn_add_compress(fp, config->compression, 0);

	if (i_auth == 1)
		fprintf(fp, "auth-user-pass %s\n", "secret");

	if (config->redirect_gateway)
		fprintf(fp, "redirect-gateway def1 bypass-dhcp\n");

	fprintf(fp, "persist-key\n");
	fprintf(fp, "script-security %d\n", 2);
	fprintf(fp, "writepid %s\n", CLIENT_PID_FILE);

	fprintf(fp, "up %s\n",  SCRIPT_OVPN_CLIENT);
	fprintf(fp, "down %s\n",  SCRIPT_OVPN_CLIENT);

	fprintf(fp, "\n### User params:\n");

	load_user_config(fp, CLIENT_CERT_DIR, "client.conf", forbidden_list);

	if (openvpn_config_file_publish(&fp, staged_conf, conf_file, 0644) != 0) {
		int saved_errno = errno ? errno : EIO;
		logmessage(CLIENT_LOG_NAME,
			   "cannot publish OpenVPN client configuration (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return 1;
	}

	return 0;
}

static int
openvpn_tapif_start(const char *ifname, int is_server, int insert_to_bridge)
{
	int created = 0;
	int status;

	if (!is_interface_exist(ifname)) {
		status = doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname);
		if (status != 0)
			return status;
		created = 1;
	}

	if (insert_to_bridge)
		br_add_del_if(IFNAME_BR, ifname, 1);
	status = doSystem("ifconfig %s %s %s", ifname, "0.0.0.0", "promisc up");
	if (status != 0) {
		/* Roll back only a TAP device created by this invocation. */
		if (created) {
			if (insert_to_bridge)
				br_add_del_if(IFNAME_BR, ifname, 0);
			(void)doSystem("%s %s --dev %s", OPENVPN_EXE, "--rmtun", ifname);
		}
		return status;
	}
	set_vpn_balancing(ifname, is_server);
	return 0;
}

static int
openvpn_tapif_stop(const char *ifname)
{
	int status;

	if (!is_interface_exist(ifname))
		return 0;
	status = ifconfig(ifname, 0, NULL, NULL);
	if (status != 0)
		return status;
	br_add_del_if(IFNAME_BR, ifname, 0);
	return doSystem("%s %s --dev %s", OPENVPN_EXE, "--rmtun", ifname);
}

static int
openvpn_tunif_start(const char *ifname, int is_server)
{
	int status;

	if (!is_interface_exist(ifname)) {
		status = doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname);
		if (status != 0)
			return status;
	}
	set_vpn_balancing(ifname, is_server);
	return 0;
}

static int
openvpn_tunif_stop(const char *ifname)
{
	int status;

	if (!is_interface_exist(ifname))
		return 0;
	status = ifconfig(ifname, 0, NULL, NULL);
	if (status != 0)
		return status;
	return doSystem("%s %s --dev %s", OPENVPN_EXE, "--rmtun", ifname);
}

/* Map the numeric OpenVPN writepid value to a complete, current process
 * identity before any signal. This prevents a stale PID from reaching a
 * recycled process with a different start_time or argv identity. */
static enum process_presence
openvpn_read_process_identity(const char *pidfile,
			      struct process_identity *identity)
{
	char pid_text[32];
	struct stat before, after;
	struct process_snapshot snapshot;
	enum process_presence state;
	pid_t pid;
	int fd, saved_errno;
	size_t used = 0, i;
	ssize_t count;

	if (!pidfile || !identity) {
		errno = EINVAL;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	fd = open(pidfile, O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK);
	if (fd < 0) {
		if (errno == ENOENT)
			return PROCESS_PRESENCE_ABSENT;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	if (fstat(fd, &before) != 0)
		goto failed;
	if (!S_ISREG(before.st_mode) || before.st_uid != (uid_t)0 ||
	    before.st_nlink != 1 || (before.st_mode & 022) != 0 ||
	    before.st_size <= 0 || before.st_size >= (off_t)sizeof(pid_text)) {
		errno = EPERM;
		goto failed;
	}
	while (used < sizeof(pid_text)) {
		count = read(fd, pid_text + used, sizeof(pid_text) - used);
		if (count < 0) {
			if (errno == EINTR)
				continue;
			goto failed;
		}
		if (count == 0)
			break;
		used += (size_t)count;
	}
	if (fstat(fd, &after) != 0)
		goto failed;
	if (before.st_dev != after.st_dev || before.st_ino != after.st_ino ||
	    before.st_size != after.st_size || used != (size_t)after.st_size) {
		errno = EIO;
		goto failed;
	}
	if (close(fd) != 0) {
		fd = -1;
		if (!errno)
			errno = EIO;
		goto failed;
	}
	fd = -1;
	if (openvpn_process_stop_parse_pid(pid_text, used, &pid) != 0)
		return PROCESS_PRESENCE_UNKNOWN;

	state = process_inventory_scan("openvpn", &snapshot);
	if (state == PROCESS_PRESENCE_UNKNOWN) {
		saved_errno = snapshot.error ? snapshot.error : EIO;
		process_snapshot_release(&snapshot);
		errno = saved_errno;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	if (state != PROCESS_PRESENCE_RUNNING &&
	    state != PROCESS_PRESENCE_ABSENT) {
		process_snapshot_release(&snapshot);
		errno = EIO;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	for (i = 0; i < snapshot.count; i++) {
		if (snapshot.items[i].pid != pid)
			continue;
		*identity = snapshot.items[i];
		process_snapshot_release(&snapshot);
		state = process_identity_validate("openvpn", identity);
		if (state == PROCESS_PRESENCE_UNKNOWN)
			errno = EIO;
		else if (state == PROCESS_PRESENCE_ABSENT)
			errno = ESRCH;
		return state;
	}
	process_snapshot_release(&snapshot);
	errno = ESRCH;
	return PROCESS_PRESENCE_ABSENT;

failed:
	saved_errno = errno ? errno : EIO;
	(void)close(fd);
	errno = saved_errno;
	return PROCESS_PRESENCE_UNKNOWN;
}

static int
openvpn_signal_process_identity(const struct process_identity *identity,
				int signal_number)
{
	enum process_presence state;

	state = process_identity_validate("openvpn", identity);
	if (state == PROCESS_PRESENCE_ABSENT)
		return 1;
	if (state != PROCESS_PRESENCE_RUNNING) {
		errno = EIO;
		return -1;
	}
	if (kill(identity->pid, signal_number) == 0)
		return 0;
	if (errno == ESRCH)
		return 1;
	return -1;
}

static int
openvpn_wait_process_stopped(const struct process_identity *identity)
{
	enum process_presence state;
	int attempt;

	for (attempt = 0; attempt < 3; attempt++) {
		sleep(1);
		state = process_identity_validate("openvpn", identity);
		if (state == PROCESS_PRESENCE_ABSENT)
			return 0;
		if (state != PROCESS_PRESENCE_RUNNING) {
			errno = EIO;
			return -1;
		}
	}
	errno = ETIMEDOUT;
	return -1;
}

static int
openvpn_stop_pidfile_process(const char *pidfile)
{
	struct process_identity identity;
	enum process_presence state;
	int signal_status, saved_errno;

	state = openvpn_read_process_identity(pidfile, &identity);
	if (state != PROCESS_PRESENCE_RUNNING) {
		if (state == PROCESS_PRESENCE_UNKNOWN) {
			saved_errno = errno ? errno : EIO;
			return openvpn_process_stop_result(state, saved_errno);
		}
		return openvpn_process_stop_result(state, 0);
	}
	signal_status = openvpn_signal_process_identity(&identity, SIGTERM);
	if (signal_status < 0)
		return -1;
	if (signal_status > 0)
		return 0;
	if (openvpn_wait_process_stopped(&identity) == 0)
		return 0;
	if (errno != ETIMEDOUT)
		return -1;

	/* Revalidate the same start_time and argv identity immediately before
	 * escalating; never send SIGKILL to a recycled numeric PID. */
	signal_status = openvpn_signal_process_identity(&identity, SIGKILL);
	if (signal_status < 0)
		return -1;
	if (signal_status > 0)
		return 0;
	if (openvpn_wait_process_stopped(&identity) == 0)
		return 0;
	return -1;
}

static int
openvpn_server_lease_parent_is_current(void)
{
	int fd = -1, saved_errno;
	ssize_t length;
	struct stat st;
	char pid_text[32];
	pid_t parent_pid = getppid();

	if (parent_pid <= (pid_t)1) {
		errno = ESTALE;
		return 0;
	}
	fd = open(SERVER_PID_FILE, O_RDONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC);
	if (fd < 0)
		return 0;
	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != (uid_t)0 ||
	    st.st_nlink != 1 || (st.st_mode & 022) != 0 || st.st_size <= 0 ||
	    st.st_size >= (off_t)sizeof(pid_text)) {
		errno = EPERM;
		goto failed;
	}
	do {
		length = read(fd, pid_text, sizeof(pid_text));
	} while (length < 0 && errno == EINTR);
	if (length < 0)
		goto failed;
	if ((off_t)length != st.st_size) {
		errno = EIO;
		goto failed;
	}
	if (close(fd) != 0) {
		fd = -1;
		if (!errno)
			errno = EIO;
		goto failed;
	}
	fd = -1;
	return openvpn_server_lease_pid_matches(pid_text, (size_t)length,
						parent_pid) == 1;
failed:
	saved_errno = errno ? errno : EIO;
	if (fd >= 0)
		(void)close(fd);
	errno = saved_errno;
	return 0;
}

static int
openvpn_server_lease_update(const char *path, int is_add,
			    const char *ifname, const char *peer_addr_l,
			    const char *peer_addr_r, const char *common_name)
{
	FILE *source = NULL, *stage = NULL;
	char staged_path[256];
	int lock_fd = -1, source_fd = -1;
	int saved_errno, stale_callback = 0;
	int trailing_newline = 1;
	struct stat source_stat;

	staged_path[0] = '\0';
	if (!path || !path[0] || !ifname || (is_add && !ifname[0]) ||
	    !peer_addr_l || !peer_addr_l[0] || !peer_addr_r || !peer_addr_r[0] ||
	    !common_name || !common_name[0] ||
	    (ifname[0] && (strlen(ifname) >= 16 ||
			   strpbrk(ifname, " \t\r\n"))) ||
	    strlen(peer_addr_l) >= 64 || strlen(peer_addr_r) >= 64 ||
	    strlen(common_name) >= 64 ||
	    strpbrk(peer_addr_l, " \t\r\n") ||
	    strpbrk(peer_addr_r, " \t\r\n") ||
	    strpbrk(common_name, "\r\n")) {
		errno = EINVAL;
		return -1;
	}

	lock_fd = vpn_server_lease_lock_acquire(VPN_SERVER_LEASE_LOCK_FILE);
	if (lock_fd < 0)
		return -1;

	/* Check after the shared lock: stop clears under this same inode. A
	 * re-parented callback cannot publish after the stop path clears it. */
	if (!openvpn_server_lease_parent_is_current()) {
		stale_callback = 1;
		goto failed;
	}
	source_fd = open(path, O_RDONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC);
	if (source_fd < 0 && (!is_add || errno != ENOENT))
		goto failed;
	if (source_fd >= 0) {
		if (fstat(source_fd, &source_stat) != 0)
			goto failed;
		if (!S_ISREG(source_stat.st_mode) ||
		    source_stat.st_uid != geteuid() || source_stat.st_nlink != 1 ||
		    (source_stat.st_mode & 022) != 0) {
			errno = EPERM;
			goto failed;
		}
		source = fdopen(source_fd, "r");
		if (!source)
			goto failed;
		source_fd = -1;
	}
	stage = openvpn_config_file_open(path, staged_path, sizeof(staged_path));
	if (!stage)
		goto failed;

	if (source) {
		if (openvpn_server_lease_copy(source, stage, peer_addr_l, peer_addr_r,
					      !is_add, &trailing_newline) != 0)
			goto failed;
		if (fclose(source) != 0) {
			source = NULL;
			if (!errno)
				errno = EIO;
			goto failed;
		}
		source = NULL;
	}
	if (is_add) {
		if (!trailing_newline && fputc('\n', stage) == EOF)
			goto failed;
		if (fprintf(stage, "%s %s %s %s\n", ifname, peer_addr_l,
			    peer_addr_r, common_name) < 0)
			goto failed;
	}
	if (openvpn_config_file_publish(&stage, staged_path, path, 0644) != 0)
		goto failed;
	if (vpn_server_lease_lock_release(lock_fd) != 0) {
		lock_fd = -1;
		return -1;
	}
	lock_fd = -1;
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (source)
		(void)fclose(source);
	if (source_fd >= 0)
		(void)close(source_fd);
	if (stage)
		(void)fclose(stage);
	if (staged_path[0])
		(void)unlink(staged_path);
	if (lock_fd >= 0)
		(void)vpn_server_lease_lock_release(lock_fd);
	errno = saved_errno;
	return stale_callback ? -2 : -1;
}

static void
on_server_client_connect(int is_tun)
{
	char *common_name = safe_getenv("common_name");
	char *peer_addr_r = safe_getenv("trusted_ip");
	char *peer_addr_l = safe_getenv("ifconfig_pool_remote_ip");
	char *dev_ifname = safe_getenv("dev");
	const char *script_name = VPN_SERVER_UPDOWN_SCRIPT;
	int lease_status;

#if defined (USE_IPV6)
	if (!is_valid_ipv4(peer_addr_r))
		peer_addr_r = safe_getenv("trusted_ip6");
#endif
	if (strlen(dev_ifname) == 0 && is_tun >= 0)
		dev_ifname = (is_tun) ? IFNAME_SERVER_TUN : IFNAME_SERVER_TAP;
	logmessage(SERVER_LOG_NAME, "peer %s (%s) connected - local IP: %s",
		peer_addr_r, common_name, peer_addr_l);
	lease_status = openvpn_server_lease_update(VPN_SERVER_LEASE_FILE, 1, dev_ifname,
					   peer_addr_l, peer_addr_r, common_name);
	if (lease_status != 0)
		logmessage(SERVER_LOG_NAME,
			   "failed to update connected-client lease inventory (status=%d, errno=%d)",
			   lease_status, errno);
	/* A callback from a stopped/re-parented daemon is not a live session.
	 * Ordinary bookkeeping failures must not suppress the configured hook. */
	if (lease_status == -2)
		return;
	if (check_if_file_exist(script_name)) {
		char *hook_argv[] = {
			(char *)script_name, "up", dev_ifname, peer_addr_l, peer_addr_r, common_name, NULL
		};
		_eval(hook_argv, NULL, 0, NULL);
	}
}

static void
on_server_client_disconnect(int is_tun)
{
	char *common_name = safe_getenv("common_name");
	char *peer_addr_r = safe_getenv("trusted_ip");
	char *peer_addr_l = safe_getenv("ifconfig_pool_remote_ip");
	char *dev_ifname = safe_getenv("dev");
	const char *script_name = VPN_SERVER_UPDOWN_SCRIPT;
	int lease_status;
	uint64_t llsent = strtoll(safe_getenv("bytes_sent"), NULL, 10);
	uint64_t llrecv = strtoll(safe_getenv("bytes_received"), NULL, 10);

#if defined (USE_IPV6)
	if (!is_valid_ipv4(peer_addr_r))
		peer_addr_r = safe_getenv("trusted_ip6");
#endif
	if (strlen(dev_ifname) == 0 && is_tun >= 0)
		dev_ifname = (is_tun) ? IFNAME_SERVER_TUN : IFNAME_SERVER_TAP;
	logmessage(SERVER_LOG_NAME, "peer %s (%s) disconnected, sent: %llu KB, received: %llu KB",
		peer_addr_r, common_name, llsent / 1024, llrecv / 1024);
	lease_status = openvpn_server_lease_update(VPN_SERVER_LEASE_FILE, 0, dev_ifname,
					   peer_addr_l, peer_addr_r, common_name);
	if (lease_status != 0)
		logmessage(SERVER_LOG_NAME,
			   "failed to prune disconnected-client lease inventory (status=%d, errno=%d)",
			   lease_status, errno);
	if (lease_status == -2)
		return;
	if (check_if_file_exist(script_name)) {
		char *hook_argv[] = {
			(char *)script_name, "down", dev_ifname, peer_addr_l, peer_addr_r, common_name, NULL
		};
		_eval(hook_argv, NULL, 0, NULL);
	}
}

static void
call_client_script(const char *script_name, const char *arg)
{
	int i;
	const char *env;

	if (!check_if_file_exist(script_name))
		return;

	for (i = 0; i < ARRAY_SIZE(env_ovpn); i++) {
		env = env_ovpn[i];
		if (strlen(safe_getenv(env)) < 1 && env_ovpn_alt[i])
			env = env_ovpn_alt[i];
		setenv(env_pppd[i], safe_getenv(env), 1);
	}

	{
		char *script_argv[] = { (char *)script_name, (char *)arg, NULL };
		_eval(script_argv, NULL, 0, NULL);
	}

	for (i = 0; i < ARRAY_SIZE(env_ovpn); i++)
		unsetenv(env_pppd[i]);
}

static int
on_client_ifup(void)
{
#define ENV_SCAN_MAX 20
	int i, i_dns = 0, i_wins = 0, i_dom = 0, vpnc_pdns;
	int status = 0, write_status, dns_status, api_result, read_errno;
	int pdns_status;
	char *pdns_value = NULL;
	char buf[256], foption[32], fvalue[128], domain[128], *value;
	struct vpn_client_nvram_value values[2];
	unsigned int value_count = 0;

	domain[0] = '\0';
	buf[0] = 0;

	/* Do not let a failed compatibility read silently disable DNS policy. */
	errno = 0;
	api_result = nvram_get_checked("vpnc_pdns", &pdns_value);
	read_errno = errno;
	vpnc_pdns = 0;
	pdns_status = vpn_client_nvram_read_bool_status(
		api_result, read_errno, pdns_value, 0, &vpnc_pdns);
	if (pdns_status != 0) {
		status = route_status_accumulate(status, pdns_status);
		logmessage(CLIENT_LOG_NAME,
			   "cannot read OpenVPN DNS push policy; preserving existing resolver state (status=%d)",
			   pdns_status);
	}

	for (i = 0; i < ENV_SCAN_MAX; i++) {
		snprintf(foption, sizeof(foption), "foreign_option_%d", i);
		value = getenv(foption);
		if (value) {
			memset(fvalue, 0, sizeof(fvalue));
			if (sscanf(value, "dhcp-option DNS %127s", fvalue) == 1) {
				if (pdns_status == 0 && vpnc_pdns > 0) {
					int buf_len = strlen(buf);
					snprintf(buf + buf_len, sizeof(buf) - buf_len, "%s%s", (buf_len) ? " " : "", fvalue);
				}
				i_dns++;
				if (i_dns == 1)
					setenv("DNS1", fvalue, 1);
				else if (i_dns == 2)
					setenv("DNS2", fvalue, 1);
			} else if (sscanf(value, "dhcp-option WINS %127s", fvalue) == 1) {
				i_wins++;
				if (i_wins == 1)
					setenv("WINS1", fvalue, 1);
				else if (i_wins == 2)
					setenv("WINS2", fvalue, 1);
			} else if (sscanf(value, "dhcp-option DOMAIN %127s", fvalue) == 1) {
				i_dom++;
				if (i_dom == 1) {
					if (pdns_status == 0 && vpnc_pdns > 0)
						snprintf(domain, sizeof(domain), "%s", fvalue);
					setenv("DOMAIN", fvalue, 1);
				}
			}
		}
		if (i_dom >= 1 && i_wins >= 2 && i_dns >= 2)
			break;
	}

	if (pdns_status == 0) {
		values[value_count].name = "vpnc_dom_t";
		values[value_count++].value =
			(vpnc_pdns > 0 && domain[0] != '\0') ? domain : "";
		values[value_count].name = "vpnc_state_t";
		values[value_count++].value = "1";
		write_status = vpn_client_nvram_set_all_values(
			values, value_count, vpn_client_nvram_set_temp_status, NULL);
	} else {
		/* Link state and the user hook are mandatory even when DNS policy is unreadable. */
		write_status = vpn_client_nvram_set_temp_status(
			NULL, "vpnc_state_t", "1");
	}
	status = route_status_accumulate(status, write_status);
	if (write_status != 0)
		logmessage(CLIENT_LOG_NAME,
			   "cannot publish OpenVPN client up/domain state (status=%d)",
			   write_status);

	if (pdns_status == 0) {
		dns_status = vpn_client_nvram_set_temp_status(NULL, "vpnc_dns_t", buf);
		status = route_status_accumulate(status, dns_status);
		if (dns_status == 0)
			refresh_resolvconf_nonfatal("OpenVPN client DNS update", 0);
		else
			logmessage(CLIENT_LOG_NAME,
				   "cannot publish OpenVPN client DNS snapshot (status=%d)",
				   dns_status);
	}

	call_client_script(VPN_CLIENT_UPDOWN_SCRIPT, "up");

	if (i_dom > 0)
		unsetenv("DOMAIN");
	if (i_wins > 1)
		unsetenv("WINS2");
	if (i_wins > 0)
		unsetenv("WINS1");
	if (i_dns > 1)
		unsetenv("DNS2");
	if (i_dns > 0)
		unsetenv("DNS1");

	return status;
}
static int
on_client_ifdown(void)
{
	int status, dns_status;

	status = vpn_client_nvram_set_temp_status(NULL, "vpnc_state_t", "0");
	if (status != 0)
		logmessage(CLIENT_LOG_NAME,
			   "cannot clear OpenVPN client up state (status=%d)", status);
	dns_status = restore_dns_from_vpnc();
	status = route_status_accumulate(status, dns_status);
	if (dns_status != 0)
		logmessage(CLIENT_LOG_NAME,
			   "cannot restore DNS after OpenVPN client disconnect (status=%d)",
			   dns_status);

	call_client_script(VPN_CLIENT_UPDOWN_SCRIPT, "down");
	return status;
}

int
start_openvpn_server(void)
{
	struct openvpn_server_network_config network;
	struct openvpn_server_options_config options;
	int interface_status;
	char vpns_cfg[64], vpns_scr[64];
	char *server_conf = "server.conf";
	char *openvpn_argv[] = {
		OPENVPN_EXE,
		"--daemon", "openvpn-srv",
		"--cd", SERVER_ROOT_DIR,
		"--config", server_conf,
		NULL
	};

	snprintf(vpns_cfg, sizeof(vpns_cfg), "%s/%s", SERVER_ROOT_DIR, server_conf);
	snprintf(vpns_scr, sizeof(vpns_scr), "%s/%s", SERVER_ROOT_DIR, SCRIPT_OVPN_SERVER);

	if (openvpn_read_server_network_snapshot(&network) != 0) {
		int saved_errno = errno ? errno : EIO;

		logmessage(SERVER_LOG_NAME,
			   "cannot read/validate OpenVPN server network snapshot; refusing startup (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return 1;
	}

	if (openvpn_read_server_options_snapshot(&options) != 0) {
		int saved_errno = errno ? errno : EIO;

		logmessage(SERVER_LOG_NAME,
			   "cannot read/validate OpenVPN server options; refusing startup (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return 1;
	}

	mkdir_if_none(SERVER_ROOT_DIR, "755");
	mkdir_if_none(COMMON_TEMP_DIR, "777");

	/* create conf file */
	if (openvpn_create_server_conf(vpns_cfg, &network, &options))
		return 1;

	/* create tun or tap device (and add tap to bridge) */
	if (network.is_tun)
		interface_status = openvpn_tunif_start(IFNAME_SERVER_TUN, 1);
	else
		interface_status = openvpn_tapif_start(IFNAME_SERVER_TAP, 1, 1);
	if (interface_status != 0) {
		logmessage(SERVER_LOG_NAME, "cannot prepare OpenVPN server interface (status=%d)", interface_status);
		return interface_status;
	}

	/* create script symlink */
	symlink("/sbin/rc", vpns_scr);

	logmessage(LOGNAME, "starting %s...", SERVER_LOG_NAME);

	return _eval(openvpn_argv, NULL, 0, NULL);
}

int
start_openvpn_client(void)
{
	struct openvpn_client_config_snapshot config;
	int interface_status;
	char vpnc_cfg[64], vpnc_scr[64];
	char *client_conf = "client.conf";
	char *openvpn_argv[] = {
		OPENVPN_EXE,
		"--daemon", "openvpn-cli",
		"--cd", CLIENT_ROOT_DIR,
		"--config", client_conf,
		NULL
	};

	snprintf(vpnc_cfg, sizeof(vpnc_cfg), "%s/%s", CLIENT_ROOT_DIR, client_conf);
	snprintf(vpnc_scr, sizeof(vpnc_scr), "%s/%s", CLIENT_ROOT_DIR, SCRIPT_OVPN_CLIENT);

	if (openvpn_read_client_config_snapshot(&config) != 0) {
		int saved_errno = errno ? errno : EIO;

		logmessage(CLIENT_LOG_NAME,
			   "cannot read/validate OpenVPN client configuration; refusing startup (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return 1;
	}

	doSystem("mkdir -p -m %s %s", "755", CLIENT_ROOT_DIR);

	/* create conf file */
	if (openvpn_create_client_conf(vpnc_cfg, &config))
		return 1;

	/* create tun or tap device */
	if (config.is_tun)
		interface_status = openvpn_tunif_start(IFNAME_CLIENT_TUN, 0);
	else
		interface_status = openvpn_tapif_start(IFNAME_CLIENT_TAP, 0, config.client_nat ? 0 : 1);
	if (interface_status != 0) {
		logmessage(CLIENT_LOG_NAME, "cannot prepare OpenVPN client interface (status=%d)", interface_status);
		return interface_status;
	}

	/* create script symlink */
	symlink("/sbin/rc", vpnc_scr);

	logmessage(LOGNAME, "starting %s...", CLIENT_LOG_NAME);

	return _eval(openvpn_argv, NULL, 0, NULL);
}

int
stop_openvpn_server(void)
{
	char vpns_scr[64];
	int status;

	status = openvpn_stop_pidfile_process(SERVER_PID_FILE);
	if (status != 0) {
		logmessage(SERVER_LOG_NAME,
			   "refusing OpenVPN server cleanup because process stop is incomplete (errno=%d)",
			   errno ? errno : EIO);
		return status;
	}
	status = openvpn_tapif_stop(IFNAME_SERVER_TAP);
	if (status != 0)
		return status;
	status = openvpn_tunif_stop(IFNAME_SERVER_TUN);
	if (status != 0)
		return status;
	snprintf(vpns_scr, sizeof(vpns_scr), "%s/%s", SERVER_ROOT_DIR, SCRIPT_OVPN_SERVER);
	if (unlink(vpns_scr) != 0 && errno != ENOENT)
		return -1;
	return 0;
}

int
stop_openvpn_client(void)
{
	char vpnc_scr[64];
	int status;

	status = openvpn_stop_pidfile_process(CLIENT_PID_FILE);
	if (status != 0) {
		logmessage(CLIENT_LOG_NAME,
			   "refusing OpenVPN client cleanup because process stop is incomplete (errno=%d)",
			   errno ? errno : EIO);
		return status;
	}
	status = openvpn_tapif_stop(IFNAME_CLIENT_TAP);
	if (status != 0)
		return status;
	status = openvpn_tunif_stop(IFNAME_CLIENT_TUN);
	if (status != 0)
		return status;
	snprintf(vpnc_scr, sizeof(vpnc_scr), "%s/%s", CLIENT_ROOT_DIR, SCRIPT_OVPN_CLIENT);
	if (unlink(vpnc_scr) != 0 && errno != ENOENT)
		return -1;
	return 0;
}

static int
openvpn_server_lease_clear(void)
{
	int lock_fd, result = 0, saved_errno = 0;

	lock_fd = vpn_server_lease_lock_acquire(VPN_SERVER_LEASE_LOCK_FILE);
	if (lock_fd < 0)
		return -1;
	/* Remove the daemon identity before the last-good snapshot. A callback
	 * waiting behind this lock cannot pass the PID-file fence after stop. */
	if (unlink(SERVER_PID_FILE) != 0 && errno != ENOENT) {
		result = -1;
		saved_errno = errno ? errno : EIO;
		goto out;
	}
	if (unlink(VPN_SERVER_LEASE_FILE) != 0 && errno != ENOENT) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
out:
	if (vpn_server_lease_lock_release(lock_fd) != 0 && result == 0) {
		result = -1;
		saved_errno = errno ? errno : EIO;
	}
	if (result != 0)
		errno = saved_errno;
	return result;
}

int
restart_openvpn_server(void)
{
	int restart_status, saved_errno;

	/* Note: SIGHUP is buggy with server drop privilegies. */
	restart_status = vpn_openvpn_server_restart_run(stop_openvpn_server,
						       openvpn_server_lease_clear,
						       start_openvpn_server);
	if (restart_status != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(SERVER_LOG_NAME,
			   "OpenVPN server restart aborted or failed (status=%d, errno=%d)",
			   restart_status, saved_errno);
		errno = saved_errno;
	}
	return restart_status;
}

int
ovpn_server_script_main(int argc, char **argv)
{
	int i_mode_tun = -1, api_result, read_errno, mode_status;
	char *mode_value = NULL;
	char *dev_ifname = safe_getenv("dev");
	char *script_type = safe_getenv("script_type");

	umask(0000);

	/* The OpenVPN-provided device is authoritative. Read NVRAM only for
	 * legacy callbacks that omit it; never silently turn a read error into TAP. */
	if ((strcmp(script_type, "client-connect") == 0 ||
	     strcmp(script_type, "client-disconnect") == 0) &&
	    strlen(dev_ifname) == 0) {
		errno = 0;
		api_result = nvram_get_checked("vpns_ov_mode", &mode_value);
		read_errno = errno;
		mode_status = vpn_server_config_int_status(api_result, read_errno,
							 mode_value, 0, 0, 1,
							 &i_mode_tun);
		if (mode_status != 0) {
			logmessage(SERVER_LOG_NAME,
				   "cannot read OpenVPN server mode for callback device fallback (status=%d); preserving unknown device",
				   mode_status);
			i_mode_tun = -1;
		}
	}

	if (strcmp(script_type, "client-connect") == 0)
	{
		on_server_client_connect(i_mode_tun);
	}
	else if (strcmp(script_type, "client-disconnect") == 0)
	{
		on_server_client_disconnect(i_mode_tun);
	}

	return 0;
}

int
ovpn_client_script_main(int argc, char **argv)
{
	char *script_type = safe_getenv("script_type");

	umask(0000);

	if (strcmp(script_type, "up") == 0)
		return on_client_ifup();
	else if (strcmp(script_type, "down") == 0)
		return on_client_ifdown();

	return 0;
}

int
ovpn_server_expcli_main(int argc, char **argv)
{
	FILE *fp = NULL;
	struct openvpn_server_network_config network;
	struct openvpn_server_options_config options;
	int i, i_prot, i_atls, i_tcv2, days_valid, config_status;
	int cert_status, keygen_status, env_set = 0, temp_dir_created = 0;
	int protocol_status, days_status;
#if defined (USE_IPV6)
	int wan_addr6_status;
#endif
	int saved_errno, path_len;
	const char *p_prot, *wan_addr, *rsa_bits;
	char tmp_ovpn_path[] = "/tmp/export_ovpn.XXXXXX";
	char tmp_ovpn_conf[] = "/tmp/client.ovpn";
	char staged_ovpn_conf[128] = {0};
	char tls_client_key_path[128];
#if defined (USE_IPV6)
	char addr6s[INET6_ADDRSTRLEN] = {0};
#endif
	if (argc < 2 || strlen(argv[1]) < 1) {
		printf("Usage: %s common_name [rsa_bits/ec_name] [days_valid]\n", argv[0]);
		return 1;
	}
	rsa_bits = "1024";
	if (argc > 2)
		rsa_bits = argv[2];
	days_status = openvpn_cert_days_valid_parse(
		argc > 3 ? argv[3] : NULL, 365, &days_valid);
	if (days_status != 0) {
		logmessage(SERVER_LOG_NAME,
			   "invalid OpenVPN client certificate validity period (status=%d)",
			   days_status);
		return 1;
	}

	/* Match the exported profile to the currently validated server settings. */
	config_status = openvpn_read_server_network_snapshot(&network);
	if (config_status == 0)
		config_status = openvpn_read_server_options_snapshot(&options);
	if (config_status != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(SERVER_LOG_NAME,
			   "cannot read OpenVPN server settings for client export (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return 1;
	}
	i_atls = options.tls_auth_mode;
	i_tcv2 = options.tls_crypt_v2;
	for (i=0; i<6; i++) {
		if (!i_atls && (i == 4))
			continue;
		if (!i_tcv2 && (i == 5))
			continue;
		if (!openvpn_check_key(openvpn_server_keys[i], 1)) {
			printf("Error: server file %s is not found\n", openvpn_server_keys[i]);
			return 1;
		}
	}
	i_prot = options.protocol;
	protocol_status = openvpn_resolve_protocol_family(
		&i_prot, "client profile export");
	if (protocol_status != 0)
		return 1;

	if (unlink(tmp_ovpn_conf) != 0 && errno != ENOENT) {
		logmessage(SERVER_LOG_NAME,
			   "cannot remove previous OpenVPN client export (errno=%d)", errno);
		return 1;
	}
	if (!mkdtemp(tmp_ovpn_path)) {
		logmessage(SERVER_LOG_NAME,
			   "cannot create private OpenVPN export staging directory (errno=%d)",
			   errno ? errno : EIO);
		return 1;
	}
	temp_dir_created = 1;
	if (setenv("CRT_PATH_CLI", tmp_ovpn_path, 1) != 0)
		goto failed;
	env_set = 1;
	{
		char days_arg[16];
		char *cert_argv[] = {
			"/usr/bin/openvpn-cert.sh", "client", "-n", argv[1],
			"-b", (char *)rsa_bits, "-d", days_arg, NULL
		};
		snprintf(days_arg, sizeof(days_arg), "%d", days_valid);
		errno = 0;
		cert_status = _eval(cert_argv, NULL, 0, NULL);
	}
	if (unsetenv("CRT_PATH_CLI") != 0)
		goto failed;
	env_set = 0;
	if (cert_status != 0) {
		if (!errno)
			errno = EIO;
		logmessage(SERVER_LOG_NAME,
			   "OpenVPN client certificate generation failed (status=%d, errno=%d)",
			   cert_status, errno);
		goto failed;
	}
#if defined (USE_IPV6)
	if (i_prot == 3)
		p_prot = "tcp6-client";
	else if (i_prot == 2)
		p_prot = "udp6";
	else if (i_prot == 5)
		p_prot = "tcp-client";
	else if (i_prot == 4)
		p_prot = "udp";
	else
#endif
	if (i_prot == 1)
		p_prot = "tcp4-client";
	else
		p_prot = "udp4";
	wan_addr = get_ddns_fqdn();
	if (!wan_addr) {
#if defined (USE_IPV6)
		if (i_prot > 1) {
			wan_addr6_status = get_wan_addr6_host_status(addr6s, &wan_addr);
			if (wan_addr6_status != 0) {
				errno = wan_addr6_status;
				logmessage(SERVER_LOG_NAME,
					   "cannot read WAN IPv6 endpoint for OpenVPN client export (status=%d)",
					   wan_addr6_status);
				goto failed;
			}
			if (!wan_addr)
				wan_addr = get_lan_addr6_host(addr6s);
			/* Dual-stack listeners remain reachable over IPv4 when this router
			 * has no usable IPv6 endpoint. IPv6-only modes stay IPv6-only. */
			if (!wan_addr && i_prot > 3) {
				wan_addr = get_wan_unit_value(0, "ipaddr");
				if (!is_valid_ipv4(wan_addr))
					wan_addr = NULL;
			}
		} else
#endif
		{
			wan_addr = get_wan_unit_value(0, "ipaddr");
			if (!is_valid_ipv4(wan_addr))
				wan_addr = NULL;
		}
	}
	if (!wan_addr)
		wan_addr = "{wan_address}";
	fp = openvpn_config_file_open(tmp_ovpn_conf, staged_ovpn_conf,
				       sizeof(staged_ovpn_conf));
	if (!fp)
		goto failed;
	fprintf(fp, "client\n");
	fprintf(fp, "dev %s\n", network.is_tun ? "tun" : "tap");
	fprintf(fp, "proto %s\n", p_prot);
	fprintf(fp, "remote %s %d\n", wan_addr, options.port);
	fprintf(fp, "resolv-retry %s\n", "infinite");
	fprintf(fp, ";float\n");
	fprintf(fp, "nobind\n");
	fprintf(fp, "persist-key\n");
	fprintf(fp, "persist-tun\n");
	openvpn_add_auth(fp, options.auth_digest);
	openvpn_add_cipher(fp, options.cipher, options.ncp_cipher_list);
	openvpn_add_compress(fp, options.compression, 0);
	fprintf(fp, "nice %d\n", 0);
	fprintf(fp, "verb %d\n", 3);
	fprintf(fp, "mute %d\n", 10);
	fprintf(fp, "remote-cert-tls server\n");
	if (openvpn_add_key(fp, SERVER_CERT_DIR, openvpn_server_keys[0], "ca") != 0 ||
	    openvpn_add_key(fp, tmp_ovpn_path, openvpn_client_keys[1], "cert") != 0 ||
	    openvpn_add_key(fp, tmp_ovpn_path, openvpn_client_keys[2], "key") != 0)
		goto failed;
	if (i_atls == 1) {
		if (openvpn_add_key(fp, SERVER_CERT_DIR, openvpn_server_keys[4], "tls-auth") != 0)
			goto failed;
		fprintf(fp, "key-direction %d\n", 1);
	} else if (i_atls == 2) {
		if (openvpn_add_key(fp, SERVER_CERT_DIR, openvpn_server_keys[4], "tls-crypt") != 0)
			goto failed;
	}
	if (i_tcv2) {
		errno = 0;
		keygen_status = doSystem("%s --genkey tls-crypt-v2-client %s/%s --tls-crypt-v2 %s/stc2.key",
					 OPENVPN_EXE, tmp_ovpn_path, openvpn_client_keys[3],
					 SERVER_CERT_DIR);
		if (keygen_status != 0) {
			if (!errno)
				errno = EIO;
			logmessage(SERVER_LOG_NAME,
				   "OpenVPN TLS-Crypt-v2 client key generation failed (status=%d, errno=%d)",
				   keygen_status, errno);
			goto failed;
		}
		path_len = snprintf(tls_client_key_path, sizeof(tls_client_key_path),
				    "%s/%s", tmp_ovpn_path, openvpn_client_keys[3]);
		if (path_len < 0 || (size_t)path_len >= sizeof(tls_client_key_path)) {
			errno = ENAMETOOLONG;
			goto failed;
		}
		if (chmod(tls_client_key_path, S_IRUSR | S_IWUSR) != 0)
			goto failed;
		if (openvpn_add_key(fp, tmp_ovpn_path,
				    openvpn_client_keys[3], "tls-crypt-v2") != 0)
			goto failed;
	}
	if (openvpn_export_remove_tempdir(tmp_ovpn_path) != 0)
		goto failed;
	temp_dir_created = 0;
	if (openvpn_config_file_publish(&fp, staged_ovpn_conf,
					tmp_ovpn_conf, 0600) != 0)
		goto failed;
	return 0;
failed:
	saved_errno = errno ? errno : EIO;
	if (env_set && unsetenv("CRT_PATH_CLI") != 0)
		logmessage(SERVER_LOG_NAME, "failed to clear OpenVPN certificate-generation environment");
	if (fp) {
		if (fclose(fp) != 0)
			logmessage(SERVER_LOG_NAME, "failed to close staged OpenVPN client export");
		fp = NULL;
	}
	if (staged_ovpn_conf[0] &&
	    unlink(staged_ovpn_conf) != 0 && errno != ENOENT)
		logmessage(SERVER_LOG_NAME, "failed to remove staged OpenVPN client export");
	if (temp_dir_created)
		(void)openvpn_export_remove_tempdir(tmp_ovpn_path);
	errno = saved_errno;
	logmessage(SERVER_LOG_NAME, "OpenVPN client export failed (errno=%d)", saved_errno);
	return 1;
}

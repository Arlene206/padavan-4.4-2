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

#define OPENVPN_EXE		"/usr/sbin/openvpn"
#define COMMON_TEMP_DIR		"/tmp/openvpn"

#define SERVER_PID_FILE		"/var/run/openvpn_svr.pid"
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

static int
openvpn_create_server_acl(FILE *fp, const char *ccd, unsigned int vnet, unsigned int vmsk)
{
	int i, i_max, ccd_len;
	char *acl_user, *acl_rnet, *acl_rmsk;
	char acl_user_var[16], acl_rnet_var[16], acl_rmsk_var[16], vpns_ccd[64];
	struct stat ccd_stat;

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

	i_max = nvram_get_int("vpns_num_x");
	if (i_max > MAX_CLIENTS_NUM)
		i_max = MAX_CLIENTS_NUM;

	for (i = 0; i < i_max; i++) {
		snprintf(acl_user_var, sizeof(acl_user_var), "vpns_user_x%d", i);
		snprintf(acl_rnet_var, sizeof(acl_rnet_var), "vpns_rnet_x%d", i);
		snprintf(acl_rmsk_var, sizeof(acl_rmsk_var), "vpns_rmsk_x%d", i);
		acl_user = nvram_safe_get(acl_user_var);
		acl_rnet = nvram_safe_get(acl_rnet_var);
		acl_rmsk = nvram_safe_get(acl_rmsk_var);
		if (openvpn_safe_path_component(acl_user) &&
		    is_valid_ipv4(acl_rnet) && is_valid_ipv4(acl_rmsk)) {
			FILE *fp_ccf;
			char ccf[80], staged_ccf[128];
			int ccf_len;

			ccf_len = snprintf(ccf, sizeof(ccf), "%s/%s", vpns_ccd, acl_user);
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
			{
				char acl_addr_var[16];
				struct in_addr pool_in;
				unsigned int vp_a;

				snprintf(acl_addr_var, sizeof(acl_addr_var), "vpns_addr_x%d", i);
				vp_a = (unsigned int)nvram_get_int(acl_addr_var);
				if (vp_a > 1 && vp_a < 255) {
					pool_in.s_addr = htonl((vnet & vmsk) | vp_a);
					fprintf(fp_ccf, "ifconfig-push %s %s\n",
						inet_ntoa(pool_in), VPN_SERVER_SUBNET_MASK);
					fprintf(fp, "route %s %s %s\n",
						acl_rnet, acl_rmsk, inet_ntoa(pool_in));
				}
				fprintf(fp_ccf, "iroute %s %s\n", acl_rnet, acl_rmsk);
			}
			if (openvpn_config_file_publish(&fp_ccf, staged_ccf, ccf, 0644) != 0) {
				logmessage(SERVER_LOG_NAME,
					   "cannot publish client-config file %s (errno=%d)", ccf, errno);
				return 0;
			}
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

static void
openvpn_add_key(FILE *fp, const char *key_dir, const char *key_file, const char *key_sect)
{
	FILE *fpk;
	int skip_head = 1;
	char buff[MAX_FILE_LINE_SIZE] = {0};

	snprintf(buff, sizeof(buff), "%s/%s", key_dir , key_file);
	fpk = fopen(buff, "r");
	if (fpk) {
		fprintf(fp, "<%s>\n", key_sect);
		while (fgets(buff, sizeof(buff), fpk) != NULL) {
			if (skip_head) {
				if (!strstr(buff, "-----BEGIN "))
					continue;
				skip_head = 0;
			}
			fputs(buff, fp);
		}
		fprintf(fp, "</%s>\n", key_sect);
		fclose(fpk);
	}
}

static int
openvpn_create_server_conf(const char *conf_file, int is_tun)
{
	FILE *fp;
	char staged_conf[128];
	int i, i_prot, i_prot_ori, i_atls, i_tcv2, i_rdgw, i_dhcp, i_items;
	unsigned int laddr, lmask;
	char *lanip, *lannm, *wins, *dns1, *dns2;
	const char *p_prot;
	struct in_addr pool_in;

	i_atls = nvram_get_int("vpns_ov_atls");
	i_tcv2 = nvram_get_int("vpns_ov_tcv2");

	for (i=0; i<6; i++) {
		if (!i_atls && (i == 4))
			continue;
		if (!i_tcv2 && (i == 5))
			continue;
		if (!openvpn_check_key(openvpn_server_keys[i], 1))
			return 1;
	}

	i_prot = nvram_get_int("vpns_ov_prot");
	i_rdgw = nvram_get_int("vpns_ov_rdgw");

	i_dhcp = is_dhcpd_enabled(0);

	lanip = nvram_safe_get("lan_ipaddr");
	lannm = nvram_safe_get("lan_netmask");

	laddr = ntohl(inet_addr(lanip));
	lmask = ntohl(inet_addr(lannm));

	i_prot_ori = i_prot;
	if (i_prot > 1 && get_ipv6_type() == IPV6_DISABLED)
		i_prot &= 1;

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

	/* fixup ipv4/ipv6 mismatch */
	if (i_prot != i_prot_ori)
		nvram_set_int("vpns_ov_prot", i_prot);

	fp = openvpn_config_file_open(conf_file, staged_conf,
				       sizeof(staged_conf));
	if (!fp)
		return 1;

	fprintf(fp, "proto %s\n", p_prot);
	fprintf(fp, "port %d\n", nvram_safe_get_int("vpns_ov_port", 1194, 1, 65535));

	if (is_tun) {
		unsigned int vnet, vmsk;
		
		vnet = ntohl(inet_addr(nvram_safe_get("vpns_vnet")));
		vmsk = ntohl(inet_addr(VPN_SERVER_SUBNET_MASK));
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
		vp_b = (unsigned int)nvram_safe_get_int("vpns_cli0", 245, 1, 254);
		vp_e = (unsigned int)nvram_safe_get_int("vpns_cli1", 254, 2, 254);
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

	openvpn_add_auth(fp, nvram_get_int("vpns_ov_mdig"));
	openvpn_add_cipher(fp, nvram_get_int("vpns_ov_ciph"), nvram_get("vpns_ov_ncp_clist"));
	openvpn_add_compress(fp, nvram_get_int("vpns_ov_compress"), 1);

	i_items = 0;
	if (i_rdgw) {
		fprintf(fp, "push \"redirect-gateway def1 %s\"\n", "bypass-dhcp");
		
		if (i_dhcp) {
			dns1 = nvram_safe_get("dhcp_dns1_x");
			dns2 = nvram_safe_get("dhcp_dns2_x");
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
		wins = nvram_safe_get("dhcp_wins_x");
		if (is_valid_ipv4(wins)) {
			i_items++;
			fprintf(fp, "push \"dhcp-option %s %s\"\n", "WINS", wins);
		}
	}

#if defined(APP_SMBD) || defined(APP_NMBD)
	if ((i_items < 1) && nvram_get_int("wins_enable"))
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
openvpn_create_client_conf(const char *conf_file, int is_tun)
{
	FILE *fp;
	char staged_conf[128];
	int i, i_prot, i_prot_ori, i_auth, i_atls;
	const char *p_peer, *p_prot;

	i_auth = nvram_get_int("vpnc_ov_auth");
	i_atls = nvram_get_int("vpnc_ov_atls");

	for (i=0; i<4; i++) {
		if (i_auth == 1 && (i == 1 || i == 2))
			continue;
		if (!i_atls && (i == 3))
			continue;
		if (!openvpn_check_key(openvpn_client_keys[i], 0))
			return 1;
	}

	i_prot = nvram_get_int("vpnc_ov_prot");
	i_prot_ori = i_prot;
	if (i_prot > 1 && get_ipv6_type() == IPV6_DISABLED)
		i_prot &= 1;

	p_peer = nvram_safe_get("vpnc_peer");
	if (!openvpn_safe_remote_host(p_peer)) {
		logmessage(CLIENT_LOG_NAME, "Invalid VPN server address.");
		return 1;
	}

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

	/* fixup ipv4/ipv6 mismatch */
	if (i_prot != i_prot_ori)
		nvram_set_int("vpnc_ov_prot", i_prot);

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
	fprintf(fp, "remote %s %d\n", p_peer, nvram_safe_get_int("vpnc_ov_port", 1194, 1, 65535));
	fprintf(fp, "resolv-retry %s\n", "infinite");
	fprintf(fp, "nobind\n");

	fprintf(fp, "dev %s\n", (is_tun) ? IFNAME_CLIENT_TUN : IFNAME_CLIENT_TAP);

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

	openvpn_add_auth(fp, nvram_get_int("vpnc_ov_mdig"));
	openvpn_add_cipher(fp, nvram_get_int("vpnc_ov_ciph"), nvram_get("vpnc_ov_ncp_clist"));
	openvpn_add_compress(fp, nvram_get_int("vpnc_ov_compress"), 0);

	if (i_auth == 1)
		fprintf(fp, "auth-user-pass %s\n", "secret");

	if (nvram_match("vpnc_dgw", "1"))
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

static void
openvpn_tapif_start(const char *ifname, int is_server, int insert_to_bridge)
{
	if (!is_interface_exist(ifname))
		doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname);

	if (insert_to_bridge)
		br_add_del_if(IFNAME_BR, ifname, 1);
	doSystem("ifconfig %s %s %s", ifname, "0.0.0.0", "promisc up");
	set_vpn_balancing(ifname, is_server);
}

static void
openvpn_tapif_stop(const char *ifname)
{
	if (is_interface_exist(ifname)) {
		ifconfig(ifname, 0, NULL, NULL);
		br_add_del_if(IFNAME_BR, ifname, 0);
		doSystem("%s %s --dev %s", OPENVPN_EXE, "--rmtun", ifname);
	}
}

static void
openvpn_tunif_start(const char *ifname, int is_server)
{
	if (!is_interface_exist(ifname))
		doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname);
	set_vpn_balancing(ifname, is_server);
}

static void
openvpn_tunif_stop(const char *ifname)
{
	if (is_interface_exist(ifname)) {
		ifconfig(ifname, 0, NULL, NULL);
		doSystem("%s %s --dev %s", OPENVPN_EXE, "--rmtun", ifname);
	}
}

static int
openvpn_server_lease_lock(const char *lease_path)
{
	char lock_path[256];
	struct stat st;
	struct flock lock;
	int lock_fd, path_len, lock_result;

	path_len = snprintf(lock_path, sizeof(lock_path), "%s.lock", lease_path);
	if (path_len < 0 || path_len >= (int)sizeof(lock_path)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	lock_fd = open(lock_path, O_CREAT | O_RDWR | O_NOFOLLOW | O_CLOEXEC, 0600);
	if (lock_fd < 0)
		return -1;
	if (fstat(lock_fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() || st.st_nlink != 1) {
		errno = EPERM;
		goto failed;
	}
	if (fchmod(lock_fd, 0600) != 0)
		goto failed;
	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	do {
		lock_result = fcntl(lock_fd, F_SETLKW, &lock);
	} while (lock_result < 0 && errno == EINTR);
	if (lock_result < 0)
		goto failed;
	return lock_fd;

failed:
	{
		int saved_errno = errno ? errno : EIO;
		close(lock_fd);
		errno = saved_errno;
	}
	return -1;
}

static int
openvpn_server_lease_update(const char *path, int is_add,
			    const char *ifname, const char *peer_addr_l,
			    const char *peer_addr_r, const char *common_name)
{
	FILE *source = NULL, *stage = NULL;
	char staged_path[256];
	int lock_fd = -1, saved_errno;
	int trailing_newline = 1;

	staged_path[0] = '\0';
	if (!path || !path[0] || !ifname || !ifname[0] ||
	    !peer_addr_l || !peer_addr_l[0] || !peer_addr_r || !peer_addr_r[0] ||
	    !common_name || !common_name[0] ||
	    strlen(ifname) >= 16 || strlen(peer_addr_l) >= 64 ||
	    strlen(peer_addr_r) >= 64 || strlen(common_name) >= 64 ||
	    strpbrk(ifname, " \t\r\n") ||
	    strpbrk(peer_addr_l, " \t\r\n") ||
	    strpbrk(peer_addr_r, " \t\r\n") ||
	    strpbrk(common_name, "\r\n")) {
		errno = EINVAL;
		return -1;
	}

	lock_fd = openvpn_server_lease_lock(path);
	if (lock_fd < 0)
		return -1;
	source = fopen(path, "r");
	if (!source && (!is_add || errno != ENOENT))
		goto failed;
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
	close(lock_fd);
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (source)
		(void)fclose(source);
	if (stage)
		(void)fclose(stage);
	if (staged_path[0])
		(void)unlink(staged_path);
	if (lock_fd >= 0)
		close(lock_fd);
	errno = saved_errno;
	return -1;
}

static void
on_server_client_connect(int is_tun)
{
	char *common_name = safe_getenv("common_name");
	char *peer_addr_r = safe_getenv("trusted_ip");
	char *peer_addr_l = safe_getenv("ifconfig_pool_remote_ip");
	char *dev_ifname = safe_getenv("dev");
	const char *script_name = VPN_SERVER_UPDOWN_SCRIPT;

#if defined (USE_IPV6)
	if (!is_valid_ipv4(peer_addr_r))
		peer_addr_r = safe_getenv("trusted_ip6");
#endif
	if (strlen(dev_ifname) == 0)
		dev_ifname = (is_tun) ? IFNAME_SERVER_TUN : IFNAME_SERVER_TAP;
	logmessage(SERVER_LOG_NAME, "peer %s (%s) connected - local IP: %s",
		peer_addr_r, common_name, peer_addr_l);
	if (openvpn_server_lease_update(VPN_SERVER_LEASE_FILE, 1, dev_ifname,
					peer_addr_l, peer_addr_r, common_name) != 0)
		logmessage(SERVER_LOG_NAME, "failed to update connected-client lease inventory (errno=%d)",
			   errno);
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
	uint64_t llsent = strtoll(safe_getenv("bytes_sent"), NULL, 10);
	uint64_t llrecv = strtoll(safe_getenv("bytes_received"), NULL, 10);

#if defined (USE_IPV6)
	if (!is_valid_ipv4(peer_addr_r))
		peer_addr_r = safe_getenv("trusted_ip6");
#endif
	if (strlen(dev_ifname) == 0)
		dev_ifname = (is_tun) ? IFNAME_SERVER_TUN : IFNAME_SERVER_TAP;
	logmessage(SERVER_LOG_NAME, "peer %s (%s) disconnected, sent: %llu KB, received: %llu KB",
		peer_addr_r, common_name, llsent / 1024, llrecv / 1024);
	if (openvpn_server_lease_update(VPN_SERVER_LEASE_FILE, 0, dev_ifname,
					peer_addr_l, peer_addr_r, common_name) != 0)
		logmessage(SERVER_LOG_NAME, "failed to prune disconnected-client lease inventory (errno=%d)",
			   errno);
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

static void
on_client_ifup(void)
{
#define ENV_SCAN_MAX 20
	int i, i_dns = 0, i_wins = 0, i_dom = 0, vpnc_pdns;
	char buf[256], foption[32], fvalue[128], *value;

	nvram_set_int_temp("vpnc_state_t", 1);

	vpnc_pdns = nvram_get_int("vpnc_pdns");

	buf[0] = 0;
	for (i = 0; i < ENV_SCAN_MAX; i++) {
		snprintf(foption, sizeof(foption), "foreign_option_%d", i);
		value = getenv(foption);
		if (value) {
			memset(fvalue, 0, sizeof(fvalue));
			if (sscanf(value, "dhcp-option DNS %127s", fvalue) == 1) {
				if (vpnc_pdns > 0) {
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
					if (vpnc_pdns > 0)
						nvram_set_temp("vpnc_dom_t", fvalue);
					setenv("DOMAIN", fvalue, 1);
				}
			}
		}
		if (i_dom >= 1 && i_wins >= 2 && i_dns >= 2)
			break;
	}

	nvram_set_temp("vpnc_dns_t", buf);
	if (strlen(buf) > 0)
		refresh_resolvconf_nonfatal("OpenVPN client DNS update", 0);

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
}

static void
on_client_ifdown(void)
{
	nvram_set_int_temp("vpnc_state_t", 0);

	restore_dns_from_vpnc();

	call_client_script(VPN_CLIENT_UPDOWN_SCRIPT, "down");
}

int
start_openvpn_server(void)
{
	int i_mode_tun;
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

	mkdir_if_none(SERVER_ROOT_DIR, "755");
	mkdir_if_none(COMMON_TEMP_DIR, "777");

	i_mode_tun = (nvram_get_int("vpns_ov_mode") == 1) ? 1 : 0;

	/* create conf file */
	if (openvpn_create_server_conf(vpns_cfg, i_mode_tun))
		return 1;

	/* create tun or tap device (and add tap to bridge) */
	if (i_mode_tun)
		openvpn_tunif_start(IFNAME_SERVER_TUN, 1);
	else
		openvpn_tapif_start(IFNAME_SERVER_TAP, 1, 1);

	/* create script symlink */
	symlink("/sbin/rc", vpns_scr);

	logmessage(LOGNAME, "starting %s...", SERVER_LOG_NAME);

	return _eval(openvpn_argv, NULL, 0, NULL);
}

int
start_openvpn_client(void)
{
	int i_mode_tun;
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

	doSystem("mkdir -p -m %s %s", "755", CLIENT_ROOT_DIR);

	i_mode_tun = (nvram_get_int("vpnc_ov_mode") == 1) ? 1 : 0;

	/* create conf file */
	if (openvpn_create_client_conf(vpnc_cfg, i_mode_tun))
		return 1;

	/* create tun or tap device */
	if (i_mode_tun)
		openvpn_tunif_start(IFNAME_CLIENT_TUN, 0);
	else
		openvpn_tapif_start(IFNAME_CLIENT_TAP, 0, (nvram_get_int("vpnc_ov_cnat") == 1) ? 0 : 1);

	/* create script symlink */
	symlink("/sbin/rc", vpnc_scr);

	logmessage(LOGNAME, "starting %s...", CLIENT_LOG_NAME);

	return _eval(openvpn_argv, NULL, 0, NULL);
}

void
stop_openvpn_server(void)
{
	char vpns_scr[64];

	kill_process_pidfile(SERVER_PID_FILE, 3, 1);

	/* remove tap device */
	openvpn_tapif_stop(IFNAME_SERVER_TAP);

	/* remove tun device */
	openvpn_tunif_stop(IFNAME_SERVER_TUN);

	/* remove script symlink */
	snprintf(vpns_scr, sizeof(vpns_scr), "%s/%s", SERVER_ROOT_DIR, SCRIPT_OVPN_SERVER);
	unlink(vpns_scr);
}

void
stop_openvpn_client(void)
{
	char vpnc_scr[64];

	kill_process_pidfile(CLIENT_PID_FILE, 3, 1);

	/* remove tap device */
	openvpn_tapif_stop(IFNAME_CLIENT_TAP);

	/* remove tun device */
	openvpn_tunif_stop(IFNAME_CLIENT_TUN);

	/* remove script symlink */
	snprintf(vpnc_scr, sizeof(vpnc_scr), "%s/%s", CLIENT_ROOT_DIR, SCRIPT_OVPN_CLIENT);
	unlink(vpnc_scr);
}

void
restart_openvpn_server(void)
{
	/* Note: SIGHUP is buggy with server drop privilegies */

	stop_openvpn_server();
	unlink(VPN_SERVER_LEASE_FILE);
	start_openvpn_server();
}

int
ovpn_server_script_main(int argc, char **argv)
{
	int i_mode_tun;
	char *script_type = safe_getenv("script_type");

	i_mode_tun = (nvram_get_int("vpns_ov_mode") == 1) ? 1 : 0;

	umask(0000);

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
	{
		on_client_ifup();
	}
	else if (strcmp(script_type, "down") == 0)
	{
		on_client_ifdown();
	}

	return 0;
}

int
ovpn_server_expcli_main(int argc, char **argv)
{
	FILE *fp;
	int i, i_prot, i_atls, i_tcv2, days_valid;
	const char *p_prot, *wan_addr, *rsa_bits;
	const char *tmp_ovpn_path = "/tmp/export_ovpn";
	const char *tmp_ovpn_conf = "/tmp/client.ovpn";
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

	days_valid = 365;
	if (argc > 3 && atoi(argv[3]) > 0)
		days_valid = atoi(argv[3]);

	i_atls = nvram_get_int("vpns_ov_atls");
	i_tcv2 = nvram_get_int("vpns_ov_tcv2");

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

	/* Generate client cert and key */
	doSystem("rm -rf %s", tmp_ovpn_path);
	setenv("CRT_PATH_CLI", tmp_ovpn_path, 1);
	{
		char days_arg[16];
		char *cert_argv[] = {
			"/usr/bin/openvpn-cert.sh", "client", "-n", argv[1],
			"-b", rsa_bits, "-d", days_arg, NULL
		};
		snprintf(days_arg, sizeof(days_arg), "%d", days_valid);
		_eval(cert_argv, NULL, 0, NULL);
	}
	unsetenv("CRT_PATH_CLI");

	i_prot = nvram_get_int("vpns_ov_prot");
	if (i_prot > 1 && get_ipv6_type() == IPV6_DISABLED)
		i_prot &= 1;
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
			wan_addr = get_wan_addr6_host(addr6s);
			if (!wan_addr)
				wan_addr = get_lan_addr6_host(addr6s);
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

	fp = fopen(tmp_ovpn_conf, "w+");
	if (!fp) {
		doSystem("rm -rf %s", tmp_ovpn_path);
		printf("Error: unable to create file %s\n", tmp_ovpn_conf);
		return 1;
	}

	fprintf(fp, "client\n");
	fprintf(fp, "dev %s\n", (nvram_get_int("vpns_ov_mode") == 1) ? "tun" : "tap");
	fprintf(fp, "proto %s\n", p_prot);
	fprintf(fp, "remote %s %d\n", wan_addr, nvram_safe_get_int("vpns_ov_port", 1194, 1, 65535));
	fprintf(fp, "resolv-retry %s\n", "infinite");
	fprintf(fp, ";float\n");
	fprintf(fp, "nobind\n");
	fprintf(fp, "persist-key\n");
	fprintf(fp, "persist-tun\n");
	openvpn_add_auth(fp, nvram_get_int("vpns_ov_mdig"));
	openvpn_add_cipher(fp, nvram_get_int("vpns_ov_ciph"), nvram_get("vpns_ov_ncp_clist"));
	openvpn_add_compress(fp, nvram_get_int("vpns_ov_compress"), 0);
	fprintf(fp, "nice %d\n", 0);
	fprintf(fp, "verb %d\n", 3);
	fprintf(fp, "mute %d\n", 10);
	fprintf(fp, ";remote-cert-tls %s\n", "server");
	openvpn_add_key(fp, SERVER_CERT_DIR, openvpn_server_keys[0], "ca");
	openvpn_add_key(fp, tmp_ovpn_path, openvpn_client_keys[1], "cert");
	openvpn_add_key(fp, tmp_ovpn_path, openvpn_client_keys[2], "key");

	if (i_atls == 1) {
		openvpn_add_key(fp, SERVER_CERT_DIR, openvpn_server_keys[4], "tls-auth");
		fprintf(fp, "key-direction %d\n", 1);
	} else if (i_atls == 2) {
		openvpn_add_key(fp, SERVER_CERT_DIR, openvpn_server_keys[4], "tls-crypt");
	}

	if (i_tcv2) {
		doSystem("/usr/sbin/openvpn --genkey tls-crypt-v2-client %s/%s --tls-crypt-v2 %s/stc2.key", tmp_ovpn_path, openvpn_client_keys[3], SERVER_CERT_DIR);
		openvpn_add_key(fp, tmp_ovpn_path, openvpn_client_keys[3], "tls-crypt-v2");
	}

	fclose(fp);

	doSystem("rm -rf %s", tmp_ovpn_path);

	doSystem("unix2dos %s", tmp_ovpn_conf);
	chmod(tmp_ovpn_conf, 0600);

	return 0;
}

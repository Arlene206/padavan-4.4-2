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
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>

#include "rc.h"
#include "eapol_config_policy.h"

int stop_auth_kabinet(void)
{
	char *svcs[] = { "lanauth", NULL };
	return kill_services(svcs, 3, 1);
}

int start_auth_kabinet(void)
{
	int ret;
	char *gateip = nvram_safe_get("wan_auth_host");
	char *passwd = nvram_safe_get("wan_auth_pass");

	ret = stop_auth_kabinet();
	if (ret != 0) {
		logmessage("lanauth", "authentication restart aborted because the previous daemon did not stop (status=%d)", ret);
		return ret;
	}

	if (strlen(passwd) < 1) {
		logmessage("lanauth", "password is empty, unable to start!");
		return -1;
	}

	if (is_valid_ipv4(gateip))
		ret = eval("/usr/sbin/lanauth", "-s", gateip, "-p", passwd);
	else
		ret = eval("/usr/sbin/lanauth", "-p", passwd);

	if (ret == 0)
		logmessage("lanauth", "start authentication...");

	return ret;
}

int stop_auth_eapol(void)
{
static int
write_wpa_config_temp(const char *config_path, const char *eap_type,
                      const char *username, const char *password,
                      int eap_algo, char *temporary_path,
                      size_t temporary_path_size)
{
    FILE *fp = NULL;
    int fd = -1, length, saved_errno;
    int created = 0;

    if (!config_path || !*config_path || !eap_type || !username ||
        !password || !temporary_path || temporary_path_size == 0) {
        errno = EINVAL;
        return EINVAL;
    }
    temporary_path[0] = '\0';
    length = snprintf(temporary_path, temporary_path_size,
                      "%s.XXXXXX", config_path);
    if (length < 0)
        return errno ? errno : EIO;
    if ((size_t)length >= temporary_path_size) {
        errno = ENAMETOOLONG;
        return ENAMETOOLONG;
    }

    fd = mkstemp(temporary_path);
    if (fd < 0)
        return errno ? errno : EIO;
    created = 1;
    if (fchmod(fd, S_IRUSR | S_IWUSR) != 0)
        goto failed;

    fp = fdopen(fd, "w");
    if (!fp)
        goto failed;
    fd = -1;

    if (fprintf(fp,
                "ctrl_interface=/var/run/wpa_supplicant\n"
                "ap_scan=0\n"
                "fast_reauth=1\n"
                "network={\n"
                "\tkey_mgmt=IEEE8021X\n"
                "\teap=%s\n"
                "\tidentity=",
                eap_type) < 0)
        goto failed;
    if (wpa_eapol_config_write_quoted(fp, username) != 0)
        goto failed;
    if (fprintf(fp, "\n\tpassword=") < 0)
        goto failed;
    if (wpa_eapol_config_write_quoted(fp, password) != 0)
        goto failed;
    if (fputc('\n', fp) == EOF)
        goto failed;

#if defined(SUPPORT_PEAP_SSL)
    if (eap_algo == 5) {
        if (fprintf(fp,
                    "\tphase1=\"peaplabel=0\"\n"
                    "\tphase2=\"auth=MSCHAPV2\"\n") < 0)
            goto failed;
    } else if (eap_algo == 4 || eap_algo == 3 ||
               eap_algo == 2 || eap_algo == 1) {
        const char *phase2_auth = "MSCHAPV2";

        if (eap_algo == 1)
            phase2_auth = "PAP";
        else if (eap_algo == 2)
            phase2_auth = "CHAP";
        else if (eap_algo == 3)
            phase2_auth = "MSCHAP";
        if (fprintf(fp,
                    "\tanonymous_identity=\"anonymous\"\n"
                    "\tphase2=\"auth=%s\"\n", phase2_auth) < 0)
            goto failed;
    }
#else
    (void)eap_algo;
#endif

    if (fprintf(fp, "\teapol_flags=0\n}\n") < 0)
        goto failed;
    if (fflush(fp) != 0 || fsync(fileno(fp)) != 0)
        goto failed;
    if (fclose(fp) != 0) {
        fp = NULL;
        goto failed;
    }
    fp = NULL;
    return 0;

failed:
    saved_errno = errno ? errno : EIO;
    if (fp)
        (void)fclose(fp);
    else if (fd >= 0)
        (void)close(fd);
    if (created)
        (void)unlink(temporary_path);
    errno = saved_errno;
    return saved_errno;
}


int start_auth_eapol(char *ifname, int unit, int eap_algo)
{
    int ret, status, cleanup_status;
    char auth_user[2048], auth_pass[2048];
    char wpa_conf_temp[128];
    const char *wpa_conf = "/etc/wpa_supplicant.conf";
    char *eap_type = "MD5";
    char *log_prefix = "EAPoL-MD5";
    char *wpa_argv[] = {"/usr/sbin/wpa_supplicant",
        "-B", "-W",
        "-D", "wired",
        "-i", ifname,
        "-c", (char *)wpa_conf,
        NULL
    };

    char *cli_argv[] = {"/usr/sbin/wpa_cli",
        "-B",
        "-i", ifname,
        "-a", SCRIPT_WPACLI_WAN,
        NULL
    };

    if (!ifname || !*ifname)
        return EINVAL;

#if defined(SUPPORT_PEAP_SSL)
    if (eap_algo == 5) {
        eap_type = "PEAP";
        log_prefix = "EAPoL-PEAP";
    } else if (eap_algo == 4 || eap_algo == 3 ||
               eap_algo == 2 || eap_algo == 1) {
        eap_type = "TTLS";
        log_prefix = "EAPoL-TTLS";
    }
#endif

    /*
     * Capture each credential into owned storage. Do not stop an active
     * supplicant if either checked NVRAM read is incomplete.
     */
    status = get_wan_unit_value_copy_status(
        unit, "auth_user", auth_user, sizeof(auth_user));
    if (status != 0) {
        logmessage("EAPoL",
                   "cannot read WAN EAP identity; preserving current authentication (status=%d)",
                   status);
        return status;
    }
    status = get_wan_unit_value_copy_status(
        unit, "auth_pass", auth_pass, sizeof(auth_pass));
    if (status != 0) {
        memset(auth_user, 0, sizeof(auth_user));
        logmessage("EAPoL",
                   "cannot read WAN EAP password; preserving current authentication (status=%d)",
                   status);
        return status;
    }

    /*
     * Generate and sync a complete private file before stopping the current
     * daemons. Its same-directory temporary name makes the final rename
     * atomic and keeps readers from seeing a partial configuration.
     */
    status = write_wpa_config_temp(wpa_conf, eap_type, auth_user,
                                   auth_pass, eap_algo, wpa_conf_temp,
                                   sizeof(wpa_conf_temp));
    memset(auth_user, 0, sizeof(auth_user));
    memset(auth_pass, 0, sizeof(auth_pass));
    if (status != 0) {
        logmessage("EAPoL",
                   "cannot generate private WPA configuration (status=%d)",
                   status);
        return status;
    }

    ret = stop_auth_eapol();
    if (ret != 0) {
        if (unlink(wpa_conf_temp) != 0 && errno != ENOENT)
            logmessage("EAPoL",
                       "cannot remove unused WPA configuration temporary file (errno=%d)",
                       errno);
        logmessage("EAPoL",
                   "authentication restart aborted because previous daemons did not stop (status=%d)",
                   ret);
        return ret;
    }

    if (rename(wpa_conf_temp, wpa_conf) != 0) {
        status = errno ? errno : EIO;
        (void)unlink(wpa_conf_temp);
        errno = status;
        logmessage("EAPoL",
                   "cannot atomically publish private WPA configuration (status=%d)",
                   status);
        return status;
    }

    ret = _eval(wpa_argv, NULL, 0, NULL);
    if (ret == 0) {
        logmessage(log_prefix, "Start authentication...");
        ret = _eval(cli_argv, NULL, 0, NULL);
        if (ret != 0) {
            logmessage("EAPoL",
                       "wpa_cli event handler failed to start (status=%d)",
                       ret);
            cleanup_status = stop_auth_eapol();
            if (cleanup_status != 0)
                logmessage("EAPoL",
                           "cannot roll back incomplete EAP authentication startup (status=%d)",
                           cleanup_status);
        }
    }

    return ret;
}

_prefix, "Start authentication...");
		
		_eval(cli_argv, NULL, 0, NULL);
	}

	return ret;
}

int wpacli_main(int argc, char **argv)
{
	if (argc < 3)
		return EINVAL;

	if (!argv[1])
		return EINVAL;

	if (strcmp(argv[2], "EAP-SUCCESS") != 0)
		logmessage("EAPoL", "Authentication: %s", argv[2]);
#if 0
	/* disable DHCP lease force renew by issues with some ISP (lease losted after force renew) */
	else if (get_wan_proto(unit) == IPV4_WAN_PROTO_IPOE_DHCP)
	{
		/* Renew DHCP lease */
		doSystem("killall %s %s", "-SIGUSR1", "udhcpc");
	}
#endif

	return 0;
}


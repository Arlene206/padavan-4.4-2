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
#include "auth_kabinet_restart_policy.h"
#include "wan_nvram_status_policy.h"

int stop_auth_kabinet(void)
{
	char *svcs[] = { "lanauth", NULL };
	return kill_services(svcs, 3, 1);
}

static int
auth_kabinet_nvram_get(void *context, const char *name, char **value)
{
	(void)context;
	return nvram_get_checked(name, value);
}

static int
auth_kabinet_read_value(void *context, const char *name,
			char *value, size_t value_size)
{
	return rc_wan_nvram_copy_value(auth_kabinet_nvram_get, NULL,
				       name, value, value_size);
}

static int
auth_kabinet_stop_service(void *context)
{
	(void)context;
	return stop_auth_kabinet();
}

#define AUTH_KABINET_LEGACY_PASSWORD_LENGTH 20

static int
auth_kabinet_write_password_file(const char *password,
                                 char *credential_path,
                                 size_t credential_path_size)
{
    char temporary_path[] = "/tmp/.lanauth-password.XXXXXX";
    size_t password_length, written;
    ssize_t count;
    int fd = -1;
    int saved_errno;

    if (!password || !credential_path || credential_path_size == 0) {
        errno = EINVAL;
        return EINVAL;
    }
    credential_path[0] = '\0';
    password_length = strnlen(password, AUTH_KABINET_LEGACY_PASSWORD_LENGTH);
    if (password_length == 0)
        return EINVAL;
    if (strlen(temporary_path) + 1 > credential_path_size)
        return ENAMETOOLONG;

    fd = mkstemp(temporary_path);
    if (fd < 0)
        return errno ? errno : EIO;
    if (fchmod(fd, S_IRUSR | S_IWUSR) != 0)
        goto failed;

    written = 0;
    while (written < password_length) {
        count = write(fd, password + written, password_length - written);
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0) {
            if (count == 0)
                errno = EIO;
            goto failed;
        }
        written += (size_t)count;
    }
    if (fsync(fd) != 0)
        goto failed;
    if (close(fd) != 0) {
        fd = -1;
        goto failed;
    }
    fd = -1;
    strcpy(credential_path, temporary_path);
    return 0;

failed:
    saved_errno = errno ? errno : EIO;
    if (fd >= 0)
        (void)close(fd);
    (void)unlink(temporary_path);
    credential_path[0] = '\0';
    errno = saved_errno;
    return saved_errno;
}

static int
auth_kabinet_start_service(void *context, const char *gateway,
			  const char *password)
{
	char credential_path[128];
	int status, ret;

	(void)context;
	status = auth_kabinet_write_password_file(
		password, credential_path, sizeof(credential_path));
	if (status != 0)
		return status;

	if (is_valid_ipv4(gateway))
		ret = eval("/usr/sbin/lanauth", "-s", gateway,
			   "-f", credential_path);
	else
		ret = eval("/usr/sbin/lanauth", "-f", credential_path);

	if (ret != 0 && unlink(credential_path) != 0 && errno != ENOENT)
		logmessage("lanauth",
			   "cannot remove unused private credential file (errno=%d)",
			   errno);
	memset(credential_path, 0, sizeof(credential_path));
	return ret;
}

int start_auth_kabinet(void)
{
	int ret;
	enum auth_kabinet_restart_stage failure_stage;
	char gateip[32], passwd[2048];

	ret = auth_kabinet_restart_with_ops(
		auth_kabinet_read_value, auth_kabinet_stop_service,
		auth_kabinet_start_service, NULL,
		gateip, sizeof(gateip), passwd, sizeof(passwd),
		&failure_stage);
	if (ret != 0) {
		switch (failure_stage) {
		case AUTH_KABINET_RESTART_STAGE_READ_GATEWAY:
			logmessage("lanauth",
				   "cannot read authentication gateway; preserving current daemon (status=%d)",
				   ret);
			break;
		case AUTH_KABINET_RESTART_STAGE_READ_PASSWORD:
			logmessage("lanauth",
				   "cannot read authentication password; preserving current daemon (status=%d)",
				   ret);
			break;
		case AUTH_KABINET_RESTART_STAGE_STOP:
			logmessage("lanauth",
				   "authentication restart aborted because the previous daemon did not stop (status=%d)",
				   ret);
			break;
		case AUTH_KABINET_RESTART_STAGE_EMPTY_PASSWORD:
			logmessage("lanauth", "password is empty, unable to start!");
			break;
		case AUTH_KABINET_RESTART_STAGE_START:
			logmessage("lanauth",
				   "authentication daemon failed to start (status=%d)", ret);
			break;
		default:
			logmessage("lanauth",
				   "authentication restart failed before daemon launch (status=%d)",
				   ret);
			break;
		}
		return ret;
	}

	logmessage("lanauth", "start authentication...");
	return 0;
}

int stop_auth_eapol(void)
{
    char *svcs[] = { "wpa_cli", "wpa_supplicant", NULL };
    return kill_services(svcs, 3, 1);
}

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


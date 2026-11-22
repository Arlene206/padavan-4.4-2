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
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "rc.h"
#include "service_start_status.h"
#include "firewall_ppp_interface_policy.h"
#include "vpn_server_lease_policy.h"
#include "vpn_server_lease_lock.h"
#include "firewall_filter_policy.h"
#include "static_route_policy.h"
#include "firewall_rule_file.h"
#include "firewall_command_status.h"
#include "firewall_restore_transaction.h"
#include "firewall_lifecycle_lock.h"
#include "firewall_module_table_policy.h"
#include "firewall_mangle_ttl_policy.h"
#include "firewall_rp_filter_policy.h"
#include "firewall_hook_file.h"
#include "firewall_nvram_match_policy.h"
#if defined (APP_ADBYBY)
#include "firewall_adbyby_restore_policy.h"
#endif

static firewall_restore_transaction_t firewall_core_restore_transaction;
static int firewall_core_restore_transaction_active;
#if defined (APP_ADBYBY)
static firewall_adbyby_restore_transaction_t firewall_adbyby_restore_transaction;
#endif

#define foreach_x(x)		for (i=0; i<nvram_get_int(x); i++)

#define BATTLENET_PORT		6112
#define TRANSMISSION_PPORT	51413
#define TRANSMISSION_RPORT	9091
#define ARIA_PPORT		16888
#define ARIA_RPORT		6800

#define MODULE_WEBSTR_MASK	0x01

static int
firewall_log_command_failure(const char *operation, int status)
{
	int exit_code, signal_number, saved_errno;
	int error_number;

	saved_errno = (status == -1) ? errno : 0;
	error_number = firewall_command_status_to_errno(status, saved_errno);
	if (error_number == 0)
		return 0;

	if (status == -1) {
		logmessage(LOGNAME, "%s: system() failed (errno=%d)", operation, saved_errno);
		return error_number;
	}

	exit_code = firewall_command_status_exit_code(status);
	if (exit_code >= 0) {
		logmessage(LOGNAME, "%s: command exited with status %d", operation, exit_code);
		return error_number;
	}

	signal_number = firewall_command_status_signal(status);
	if (signal_number > 0) {
		logmessage(LOGNAME, "%s: command terminated by signal %d", operation, signal_number);
		return error_number;
	}

	logmessage(LOGNAME, "%s: command failed with wait status %d", operation, status);
	return error_number;
}

#if defined (APP_ADBYBY)
static int
firewall_adbyby_blockip_preflight(void *context)
{
	int status;

	(void)context;
	/* Rebuild the complete set before restoring rules that reference it.
	 * The script swaps a fully populated replacement into the stable name. */
	status = doSystem("sh /usr/bin/adbyby.sh B");
	if (!firewall_command_status_ok(status))
		return status;
	return doSystem("ipset list blockip >/dev/null 2>&1");
}

static int
firewall_adbyby_restore_snapshot_command(void *context)
{
	const char *snapshot = (const char *)context;
	return doSystem("iptables-restore -n %s", snapshot);
}

static void
firewall_adbyby_report_restore_failure(const char *operation, int status,
				       void *context)
{
	(void)context;
	firewall_log_command_failure(operation, status);
	if (strcmp(operation, "Adbyby blockip ipset preflight") == 0)
		logmessage(LOGNAME,
			   "Skipping Adbyby firewall restore: blockip refresh failed or the set is unavailable");
}
#endif


typedef struct {
	const char *operation;
} firewall_restore_command_context_t;

static void
firewall_snapshot_remove_incomplete(const char *operation, char *path)
{
	int saved_errno;

	if (!path || !*path)
		return;
	if (unlink(path) == 0 || errno == ENOENT) {
		path[0] = '\0';
		return;
	}
	saved_errno = errno;
	logmessage(LOGNAME, "%s: cannot remove incomplete private snapshot %s (errno=%d)",
		   operation, path, saved_errno);
	errno = saved_errno;
}

static int
firewall_snapshot_core_table(const char *family, const char *table,
			     char *path, size_t path_size, void *context)
{
	char operation[128];
	int length, fd, fd_flags, status, saved_errno;

	(void)context;
	if (!path || path_size == 0) {
		errno = EINVAL;
		return -1;
	}
	length = snprintf(path, path_size, "/tmp/firewall-snapshot.XXXXXX");
	if (length < 0 || (size_t)length >= path_size) {
		path[0] = '\0';
		errno = ENAMETOOLONG;
		return -1;
	}
	fd = mkstemp(path);
	if (fd < 0) {
		saved_errno = errno;
		path[0] = '\0';
		snprintf(operation, sizeof(operation), "Firewall %s %s snapshot", family, table);
		logmessage(LOGNAME, "%s: cannot create private snapshot (errno=%d)", operation, saved_errno);
		errno = saved_errno;
		return -1;
	}
	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0) {
		saved_errno = errno;
		close(fd);
		snprintf(operation, sizeof(operation), "Firewall %s %s snapshot", family, table);
		firewall_snapshot_remove_incomplete(operation, path);
		logmessage(LOGNAME, "%s: cannot secure private snapshot (errno=%d)", operation, saved_errno);
		errno = saved_errno;
		return -1;
	}
	fd_flags = fcntl(fd, F_GETFD, 0);
	if (fd_flags < 0 ||
	    fcntl(fd, F_SETFD, fd_flags & ~FD_CLOEXEC) != 0) {
		saved_errno = errno;
		close(fd);
		snprintf(operation, sizeof(operation), "Firewall %s %s snapshot", family, table);
		firewall_snapshot_remove_incomplete(operation, path);
		logmessage(LOGNAME, "%s: cannot prepare private snapshot writer (errno=%d)",
			   operation, saved_errno);
		errno = saved_errno;
		return -1;
	}
	snprintf(operation, sizeof(operation), "Firewall %s %s snapshot", family, table);
	/* Keep mkstemp's inode open across system(); the shell duplicates this
	 * descriptor instead of reopening a pathname in the shared /tmp directory. */
	status = doSystem("%s-save -t %s >&%d", family, table, fd);
	if (!firewall_command_status_ok(status))
		firewall_log_command_failure(operation, status);
	if (close(fd) != 0) {
		saved_errno = errno;
		firewall_snapshot_remove_incomplete(operation, path);
		logmessage(LOGNAME, "%s: cannot close private snapshot after writer (errno=%d)",
			   operation, saved_errno);
		errno = saved_errno;
		return -1;
	}
	if (!firewall_command_status_ok(status)) {
		firewall_snapshot_remove_incomplete(operation, path);
		return -1;
	}
	return 0;
}

static int
firewall_restore_core_table(const char *family, const char *table,
			    const char *path, int rollback, void *context)
{
	const firewall_restore_command_context_t *command_context;
	char operation[128];
	int status;

	command_context = (const firewall_restore_command_context_t *)context;
	if (rollback)
		snprintf(operation, sizeof(operation), "Firewall %s %s rollback", family, table);
	else if (command_context && command_context->operation)
		snprintf(operation, sizeof(operation), "%s", command_context->operation);
	else
		snprintf(operation, sizeof(operation), "Firewall %s %s restore", family, table);
	status = doSystem("%s-restore %s", family, path);
	firewall_log_command_failure(operation, status);
	return firewall_command_status_ok(status) ? 0 : -1;
}

static int
firewall_remove_restore_snapshot(const char *path, void *context)
{
	(void)context;
	if (unlink(path) == 0 || errno == ENOENT)
		return 0;
	logmessage(LOGNAME, "Cannot remove firewall snapshot %s (errno=%d)", path, errno);
	return -1;
}

#if defined (APP_ADBYBY)
static int
firewall_adbyby_snapshot_table(const char *table, char *path,
			       size_t path_size, void *context)
{
	(void)context;
	return firewall_snapshot_core_table("iptables", table, path, path_size, NULL);
}

/* Unlike the extension restore, rollback must flush and recreate the whole
 * table from the complete iptables-save snapshot. */
static int
firewall_adbyby_restore_table_command(const char *table, const char *path,
				      int rollback, void *context)
{
	(void)context;
	if (!rollback)
		return -1;
	return doSystem("iptables-restore %s", path);
}

static void
firewall_log_retained_adbyby_restore_snapshots(
	const firewall_adbyby_restore_transaction_t *tx)
{
	size_t i;

	if (!tx || !tx->rollback_failed)
		return;
	for (i = 0; i < tx->snapshot_count; ++i)
		logmessage(LOGNAME,
			   "Adbyby extension rollback incomplete; recovery snapshot retained: %s table at %s",
			   tx->snapshots[i].table, tx->snapshots[i].path);
}
#endif

static void
firewall_log_retained_restore_snapshots(const firewall_restore_transaction_t *tx)
{
	size_t i;
	int rollback_failed;

	if (!tx)
		return;
	rollback_failed = firewall_restore_transaction_rollback_failed(tx);
	for (i = 0; i < tx->snapshot_count; ++i) {
		if (rollback_failed)
			logmessage(LOGNAME,
				   "Firewall rollback incomplete; recovery snapshot retained: %s table %s at %s",
				   tx->snapshots[i].family, tx->snapshots[i].table,
				   tx->snapshots[i].path);
		else
			logmessage(LOGNAME,
				   "Firewall snapshot cleanup remains pending: %s table %s at %s",
				   tx->snapshots[i].family, tx->snapshots[i].table,
				   tx->snapshots[i].path);
	}
	for (i = 0; i < tx->cleanup_path_count; ++i)
		logmessage(LOGNAME,
			   "Firewall partial snapshot cleanup remains pending at %s",
			   tx->cleanup_paths[i]);
}

static void
firewall_abort_core_restore_after_staging_failure(void)
{
	if (firewall_core_restore_transaction_active)
		(void)firewall_restore_transaction_abort(&firewall_core_restore_transaction,
			firewall_restore_core_table, firewall_remove_restore_snapshot, NULL);
}

static int
firewall_read_enabled_setting(const char *param_name, int *enabled)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!param_name || !param_name[0] || !enabled) {
		errno = EINVAL;
		return EINVAL;
	}

	errno = 0;
	api_result = nvram_get_checked(param_name, &value);
	saved_errno = errno;
	status = firewall_nvram_match_status(api_result, saved_errno,
					     value, "1", 0, enabled);
	if (status != 0) {
		firewall_abort_core_restore_after_staging_failure();
		logmessage(LOGNAME,
			   "cannot read firewall setting '%s'; refusing ruleset publication (errno=%d)",
			   param_name, status);
		errno = status;
	}
	return status;
}

static int
firewall_read_match_setting(const char *param_name, const char *expected,
			    int missing_match, int *matches)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!param_name || !param_name[0] || !expected || !matches) {
		errno = EINVAL;
		return EINVAL;
	}

	errno = 0;
	api_result = nvram_get_checked(param_name, &value);
	saved_errno = errno;
	status = firewall_nvram_match_status(api_result, saved_errno,
					     value, expected, missing_match, matches);
	if (status != 0) {
		firewall_abort_core_restore_after_staging_failure();
		logmessage(LOGNAME,
			   "cannot read firewall setting '%s'; refusing ruleset publication (errno=%d)",
			   param_name, status);
		errno = status;
	}
	return status;
}


static int
firewall_read_not_zero_setting(const char *param_name, int *enabled)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!param_name || !param_name[0] || !enabled) {
		errno = EINVAL;
		return EINVAL;
	}

	errno = 0;
	api_result = nvram_get_checked(param_name, &value);
	saved_errno = errno;
	status = firewall_nvram_not_zero_status(api_result, saved_errno,
						 value, 1, enabled);
	if (status != 0) {
		firewall_abort_core_restore_after_staging_failure();
		logmessage(LOGNAME,
			   "cannot read firewall inverted setting '%s'; refusing ruleset publication (errno=%d)",
			   param_name, status);
		errno = status;
	}
	return status;
}

static int
firewall_read_integer_setting(const char *param_name, int missing_value, int *number)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!param_name || !param_name[0] || !number) {
		errno = EINVAL;
		return EINVAL;
	}

	errno = 0;
	api_result = nvram_get_checked(param_name, &value);
	saved_errno = errno;
	status = firewall_nvram_int_status(api_result, saved_errno,
					   value, missing_value, number);
	if (status != 0) {
		firewall_abort_core_restore_after_staging_failure();
		logmessage(LOGNAME,
			   "cannot read firewall integer setting '%s'; refusing ruleset publication (errno=%d)",
			   param_name, status);
		errno = status;
	}
	return status;
}

static int
firewall_read_vts_entry_count(int *entry_count)
{
	int status;

	if (!entry_count) {
		errno = EINVAL;
		return EINVAL;
	}

	status = firewall_read_integer_setting("vts_num_x", 0, entry_count);
	if (status != 0)
		return status;
	if (!firewall_vts_entry_count_is_safe(*entry_count)) {
		firewall_abort_core_restore_after_staging_failure();
		logmessage(LOGNAME,
			   "invalid virtual-server entry count %d; refusing firewall ruleset publication",
			   *entry_count);
		errno = ERANGE;
		return ERANGE;
	}

	return 0;
}

static int
firewall_read_url_entry_count(int *entry_count)
{
	int status;

	if (!entry_count) {
		errno = EINVAL;
		return EINVAL;
	}

	status = firewall_read_integer_setting("url_num_x", 0, entry_count);
	if (status != 0)
		return status;
	if (!firewall_url_entry_count_is_safe(*entry_count)) {
		firewall_abort_core_restore_after_staging_failure();
		logmessage(LOGNAME,
			   "invalid URL-filter entry count %d; refusing firewall ruleset publication",
			   *entry_count);
		errno = ERANGE;
		return ERANGE;
	}

	return 0;
}

static int
firewall_read_lw_entry_count(int *entry_count)
{
	int status;

	if (!entry_count) {
		errno = EINVAL;
		return EINVAL;
	}

	status = firewall_read_integer_setting("filter_lw_num_x", 0, entry_count);
	if (status != 0)
		return status;
	if (!firewall_lw_entry_count_is_safe(*entry_count)) {
		firewall_abort_core_restore_after_staging_failure();
		logmessage(LOGNAME,
			   "invalid lightweight-filter entry count %d; refusing firewall ruleset publication",
			   *entry_count);
		errno = ERANGE;
		return ERANGE;
	}

	return 0;
}

static int
firewall_read_mac_filter_entry_count(int *entry_count)
{
	int status;

	if (!entry_count) {
		errno = EINVAL;
		return EINVAL;
	}

	status = firewall_read_integer_setting("macfilter_num_x", 0, entry_count);
	if (status != 0)
		return status;
	if (!firewall_mac_filter_entry_count_is_safe(*entry_count)) {
		firewall_abort_core_restore_after_staging_failure();
		logmessage(LOGNAME,
			   "invalid MAC-filter entry count %d; refusing firewall ruleset publication",
			   *entry_count);
		errno = ERANGE;
		return ERANGE;
	}

	return 0;
}

static int
firewall_read_mac_filter_settings(int *mode, int *drop_enabled)
{
	int status;

	if (!mode || !drop_enabled) {
		errno = EINVAL;
		return EINVAL;
	}
	status = firewall_read_integer_setting("macfilter_enable_x", 0, mode);
	if (status != 0)
		return status;
	if (!firewall_mac_filter_mode_is_safe(*mode)) {
		firewall_abort_core_restore_after_staging_failure();
		logmessage(LOGNAME,
			   "invalid MAC-filter mode %d; refusing firewall ruleset publication",
			   *mode);
		errno = ERANGE;
		return ERANGE;
	}

	*drop_enabled = 0;
	if (*mode <= 0)
		return 0;
	return firewall_read_enabled_setting("fw_mac_drop", drop_enabled);
}

static int
firewall_disable_forwarding(const char *operation)
{
	int status = 0, next_status;

	next_status = set_ipv4_forward(0);
	if (next_status != 0) {
		logmessage(LOGNAME, "%s: failed to disable IPv4 forwarding (status=%d)",
			   operation, next_status);
		status = rc_first_service_start_status(status, next_status);
	}
#if defined (USE_IPV6)
	next_status = set_interface_conf_int("ipv6", "all", "forwarding", 0);
	if (next_status != 0) {
		logmessage(LOGNAME, "%s: failed to disable IPv6 forwarding (status=%d)",
			   operation, next_status);
		status = rc_first_service_start_status(status, next_status);
	}
#endif
	return status;
}

static int
firewall_apply_core_restore(const char *operation, const char *family,
			    const char *table, const char *rule_path)
{
	firewall_restore_command_context_t context;
	int standalone, result, forward_status;

	standalone = !firewall_core_restore_transaction_active;
	context.operation = operation;
	if (standalone &&
	    firewall_restore_transaction_prepare_reset(&firewall_core_restore_transaction,
		firewall_remove_restore_snapshot, NULL) != 0) {
		logmessage(LOGNAME,
			   "%s: refusing standalone firewall restore; previous recovery state is unresolved",
			   operation);
		errno = EBUSY;
		return -1;
	}
	result = firewall_restore_transaction_apply(&firewall_core_restore_transaction,
		family, table, rule_path, firewall_snapshot_core_table,
		firewall_restore_core_table, firewall_remove_restore_snapshot, &context);
	if (standalone) {
		if (result == 0)
			result = firewall_restore_transaction_finish(&firewall_core_restore_transaction,
				firewall_remove_restore_snapshot, &context);
		if (firewall_core_restore_transaction.rollback_failed ||
		    firewall_core_restore_transaction.cleanup_failed)
			firewall_log_retained_restore_snapshots(&firewall_core_restore_transaction);
		if (firewall_restore_transaction_rollback_failed(&firewall_core_restore_transaction)) {
			logmessage(LOGNAME, "Firewall rollback incomplete outside rebuild; disabling forwarding");
			forward_status = firewall_disable_forwarding("Standalone firewall rollback");
			result = rc_first_service_start_status(result, forward_status);
			if (result != 0)
				errno = result > 0 ? result : EIO;
		}
	}
	return result;
}

static int
firewall_log_hook_stage_failure(const char *operation, int error_number)
{
	if (error_number <= 0)
		error_number = EIO;
	logmessage(LOGNAME, "%s: hook validation or staging failed (errno=%d)",
		   operation, error_number);
	return error_number;
}

static int
firewall_log_hook_cleanup_failure(const char *operation, const char *path,
				  int error_number)
{
	if (error_number <= 0)
		error_number = EIO;
	logmessage(LOGNAME, "%s: cannot remove staged hook %s (errno=%d)",
		   operation, path ? path : "(unknown)", error_number);
	return error_number;
}

/* Rule-file preparation failures are distinct from restore-command failures:
 * never apply an incomplete file, and always make staging errors observable. */
static FILE *
firewall_open_rule_file(char *path, const char *operation)
{
	FILE *fp;
	int saved_errno;

	fp = firewall_rule_file_open(path);
	if (!fp) {
		firewall_abort_core_restore_after_staging_failure();
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "%s: cannot create private rule file (errno=%d)",
			   operation, saved_errno);
		errno = saved_errno;
	}
	return fp;
}

static int
firewall_finish_rule_file(FILE *fp, char *path, const char *operation)
{
	int result, saved_errno;

	result = firewall_rule_file_finish(fp, path);
	if (result != 0) {
		firewall_abort_core_restore_after_staging_failure();
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "%s: cannot flush/close private rule file (errno=%d)",
			   operation, saved_errno);
		errno = saved_errno;
	}
	return result;
}

/* state match - use xt_state (xt_conntrack more slower than xt_state) */
#define CT_STATE		"state --state"

static char *g_buf;
static char g_buf_pool[4096];
static char g_buf_empty[] = "";

static void
g_buf_init(void)
{
	g_buf = g_buf_pool;
	g_buf_pool[0] = 0;
}

static size_t
g_buf_remaining(void)
{
	if (!g_buf || g_buf < g_buf_pool ||
	    g_buf >= g_buf_pool + sizeof(g_buf_pool))
		return 0;

	return sizeof(g_buf_pool) - (size_t)(g_buf - g_buf_pool);
}

static void
g_buf_set(const char *value)
{
	size_t remaining = g_buf_remaining();

	if (remaining)
		snprintf(g_buf, remaining, "%s", value ? value : "");
}

static void
g_buf_append(const char *value)
{
	size_t remaining = g_buf_remaining();
	size_t used;

	if (!remaining)
		return;

	used = strnlen(g_buf, remaining);
	if (used >= remaining) {
		g_buf[remaining - 1] = '\0';
		return;
	}

	snprintf(g_buf + used, remaining - used, "%s", value ? value : "");
}

static char *
g_buf_alloc(char *g_buf_now)
{
	size_t remaining, len;

	if (!g_buf_now || g_buf_now < g_buf_pool ||
	    g_buf_now >= g_buf_pool + sizeof(g_buf_pool)) {
		g_buf = g_buf_pool + sizeof(g_buf_pool);
		return g_buf_empty;
	}

	remaining = (size_t)(g_buf_pool + sizeof(g_buf_pool) - g_buf_now);
	len = strnlen(g_buf_now, remaining);
	if (len >= remaining) {
		g_buf_now[remaining - 1] = '\0';
		g_buf = g_buf_pool + sizeof(g_buf_pool);
	} else {
		g_buf = g_buf_now + len + 1;
	}

	return g_buf_now;
}

static const char *
proto_conv(const char *proto_name, int idx)
{
    char nvram_name[64];
    char *value = NULL;
    int api_result, saved_errno, status, name_len;

    if (!proto_name || !proto_name[0] || idx < 0) {
        errno = EINVAL;
        return NULL;
    }
    name_len = snprintf(nvram_name, sizeof(nvram_name), "%s%d", proto_name, idx);
    if (name_len < 0 || name_len >= (int)sizeof(nvram_name)) {
        errno = ERANGE;
        return NULL;
    }

    errno = 0;
    api_result = nvram_get_checked(nvram_name, &value);
    saved_errno = errno;
    status = firewall_nvram_read_status(api_result, saved_errno, value);
    if (status != 0) {
        errno = status;
        return NULL;
    }
    if (api_result == 0)
        value = "";

    /* Missing/unrecognized values keep the historical TCP default. */
    return firewall_vts_protocol_name(value);
}

static int
firewall_read_string_setting_with_presence(const char *param_name,
					   const char **value, int *present)
{
	char *checked_value = NULL;
	int api_result, saved_errno, status;

	if (!param_name || !param_name[0] || !value) {
		errno = EINVAL;
		return EINVAL;
	}

	if (present)
		*present = 0;
	errno = 0;
	api_result = nvram_get_checked(param_name, &checked_value);
	saved_errno = errno;
	status = firewall_nvram_read_status(api_result, saved_errno, checked_value);
	if (status != 0) {
		*value = NULL;
		errno = status;
		return status;
	}

	if (present)
		*present = (api_result == 1);
	*value = (api_result == 0) ? "" : checked_value;
	errno = 0;
	return 0;
}

static int
firewall_read_string_setting(const char *param_name, const char **value)
{
	return firewall_read_string_setting_with_presence(param_name, value, NULL);
}

static int
firewall_read_indexed_string_setting_with_presence(const char *param_name,
						   int idx, const char **value,
						   int *present)
{
	char nvram_name[64];
	int name_len;

	if (!param_name || !param_name[0] || idx < 0 || !value) {
		errno = EINVAL;
		return EINVAL;
	}

	name_len = snprintf(nvram_name, sizeof(nvram_name), "%s%d", param_name, idx);
	if (name_len < 0 || name_len >= (int)sizeof(nvram_name)) {
		errno = ERANGE;
		return ERANGE;
	}

	return firewall_read_string_setting_with_presence(nvram_name, value, present);
}

static int
firewall_read_indexed_string_setting(const char *param_name, int idx,
				     const char **value)
{
	char nvram_name[64];
	int name_len;

	if (!param_name || !param_name[0] || idx < 0 || !value) {
		errno = EINVAL;
		return EINVAL;
	}

	name_len = snprintf(nvram_name, sizeof(nvram_name), "%s%d", param_name, idx);
	if (name_len < 0 || name_len >= (int)sizeof(nvram_name)) {
		errno = ERANGE;
		return ERANGE;
	}

	return firewall_read_string_setting(nvram_name, value);
}

static char *
protoflag_conv(const char *proto_name, int idx, int isFlag)
{
	const char *proto;

	if (firewall_read_indexed_string_setting(proto_name, idx, &proto) != 0)
		return NULL;

	g_buf_set("");

	if (!isFlag) {
		if (!strncasecmp(proto, "UDP", 3))
			g_buf_set("udp");
		else if (!strncasecmp(proto, "OTHER", 5))
			g_buf_set("other");
		else
			g_buf_set("tcp");
	} else {
		if (strlen(proto)>3 && !strncasecmp(proto, "TCP", 3))
			g_buf_set(proto + 4);
	}

	return (g_buf_alloc(g_buf));
}

static char *
portrange_conv(const char *port_name, int idx)
{
	const char *value;

	if (firewall_read_indexed_string_setting(port_name, idx, &value) != 0)
		return NULL;
	g_buf_set(value);

	return (g_buf_alloc(g_buf));
}

static char *
ip_conv(const char *ip_name, int idx)
{
	char itemname_arr[32];

	snprintf(itemname_arr, sizeof(itemname_arr), "%s%d", ip_name, idx);
	g_buf_set(nvram_safe_get(itemname_arr));

	return (g_buf_alloc(g_buf));
}

static char *
filter_conv(char *proto, char *flag, char *srcip, char *srcport, char *dstip, char *dstport)
{
	char newstr[128];

	g_buf_set("");

	if (strcmp(proto, "") != 0) {
		snprintf(newstr, sizeof(newstr), " -p %s", proto);
		g_buf_append(newstr);
	}

	if (strcmp(flag, "") != 0) {
		snprintf(newstr, sizeof(newstr), " --tcp-flags %s %s", flag, flag);
		g_buf_append(newstr);
	}

	if (strcmp(srcip, "") != 0) {
		if (strchr(srcip , '-'))
			snprintf(newstr, sizeof(newstr), " --src-range %s", srcip);
		else
			snprintf(newstr, sizeof(newstr), " -s %s", srcip);
		g_buf_append(newstr);
	}

	if (strcmp(srcport, "") != 0) {
		snprintf(newstr, sizeof(newstr), " --sport %s", srcport);
		g_buf_append(newstr);
	}

	if (strcmp(dstip, "") != 0) {
		if (strchr(dstip, '-'))
			snprintf(newstr, sizeof(newstr), " --dst-range %s", dstip);
		else
			snprintf(newstr, sizeof(newstr), " -d %s", dstip);
		g_buf_append(newstr);
	}

	if (strcmp(dstport, "") != 0) {
		snprintf(newstr, sizeof(newstr), " --dport %s", dstport);
		g_buf_append(newstr);
	}

	return (g_buf_alloc(g_buf));
}

static int
timematch_conv_values(char *mstr, const char *date, const char *time)
{
	const char *datestr[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char time_s[8], time_e[8];
	int i, i_time_s, i_time_e, i_full_time, comma = 0;

	mstr[0] = 0;

	/* Invalid or disabled schedules must never become unqualified all-time rules. */
	if (!firewall_schedule_is_active(date, time))
		return 0;

	strncpy(time_s, time+0, 4);
	strncpy(time_e, time+4, 4);
	time_s[4] = 0;
	time_e[4] = 0;

	i_time_s = atoi(time_s);
	i_time_e = atoi(time_e);
	i_full_time = ((i_time_s == i_time_e) || (i_time_s == 0 && i_time_e == 2359)) ? 1 : 0;

	/* check anytime */
	if (!strcmp(date, "1111111") && i_full_time)
		return 1;

	/* check whole day */
	if (i_full_time) {
		sprintf(mstr, " -m time %s", "--kerneltz");
	} else {
		const char *contiguous = "";

		/* check cross-night */
		if (i_time_s > i_time_e)
			contiguous = " --contiguous";

		sprintf(mstr, " -m time --timestart %c%c:%c%c:00 --timestop %c%c:%c%c:00%s %s",
			time[0], time[1], time[2], time[3], time[4], time[5], time[6], time[7],
			contiguous, "--kerneltz");
	}

	/* check everyday */
	if (strcmp(date, "1111111")) {
		strcat(mstr, " --weekdays ");
		for (i=0; i<7; i++) {
			if (date[i] == '1') {
				if (comma)
					strcat(mstr, ",");
				else
					comma = 1;
				strcat(mstr, datestr[i]);
			}
		}
	}

	return 1;
}

static int
timematch_conv(char *mstr, const char *nv_date, const char *nv_time)
{
	char *time, *date;

	date = nvram_get(nv_date);
	if (!date)
		date = "1111111";
	time = nvram_get(nv_time);
	if (!time)
		time = "00002359";

	return timematch_conv_values(mstr, date, time);
}

/*
 * The Lightweight Filter schedule is security-sensitive: distinguish missing
 * legacy keys from checked-read failures and copy each value before the next
 * lookup can invalidate the shared NVRAM value pointer.
 */
static int
timematch_lw_filter_conv(char *mstr)
{
	const char *date_value, *time_value;
	char date_buffer[8], time_buffer[9];
	size_t value_len;
	int date_present, time_present, status;

	if (!mstr) {
		errno = EINVAL;
		return -1;
	}
	mstr[0] = 0;

	status = firewall_read_string_setting_with_presence("filter_lw_date_x",
						    &date_value, &date_present);
	if (status != 0) {
		errno = status;
		return -1;
	}
	date_value = firewall_schedule_value_or_default(date_present, date_value,
							 "1111111");
	value_len = strlen(date_value);
	if (value_len >= sizeof(date_buffer))
		return 0;
	memcpy(date_buffer, date_value, value_len + 1);

	status = firewall_read_string_setting_with_presence("filter_lw_time_x",
						    &time_value, &time_present);
	if (status != 0) {
		errno = status;
		return -1;
	}
	time_value = firewall_schedule_value_or_default(time_present, time_value,
							 "00002359");
	value_len = strlen(time_value);
	if (value_len >= sizeof(time_buffer))
		return 0;
	memcpy(time_buffer, time_value, value_len + 1);

	return timematch_conv_values(mstr, date_buffer, time_buffer);
}

/*
 * MAC schedules use the same legacy defaults as timematch_conv(), but checked
 * reads distinguish absent keys from backend/close failures. Copy the first
 * value before the next NVRAM access invalidates the shared cache pointer.
 */
static int
firewall_read_mac_filter_schedule(char *match, int index)
{
	const char *date_value, *time_value;
	char date_buffer[8], time_buffer[9];
	size_t value_len;
	int date_present, time_present, status;

	if (!match || index < 0) {
		errno = EINVAL;
		return -1;
	}
	match[0] = '\0';

	status = firewall_read_indexed_string_setting_with_presence(
		"macfilter_date_x", index, &date_value, &date_present);
	if (status != 0) {
		errno = status;
		return -1;
	}
	date_value = firewall_schedule_value_or_default(date_present, date_value,
							"1111111");
	value_len = strlen(date_value);
	if (value_len >= sizeof(date_buffer))
		date_buffer[0] = '\0';
	else
		memcpy(date_buffer, date_value, value_len + 1);

	status = firewall_read_indexed_string_setting_with_presence(
		"macfilter_time_x", index, &time_value, &time_present);
	if (status != 0) {
		errno = status;
		return -1;
	}
	time_value = firewall_schedule_value_or_default(time_present, time_value,
							"00002359");
	value_len = strlen(time_value);
	if (value_len >= sizeof(time_buffer))
		time_buffer[0] = '\0';
	else
		memcpy(time_buffer, time_value, value_len + 1);

	return timematch_conv_values(match, date_buffer, time_buffer);
}

static char *
iprange_ex_conv(const char *ip_name, int idx)
{
	const char *ip;
	char startip[16];
	struct in_addr addr;
	size_t ip_len;
	int i, j, mask, wildcard_seen, valid;

	if (firewall_read_indexed_string_setting(ip_name, idx, &ip) != 0)
		return NULL;
	g_buf_set("");

	ip_len = strlen(ip);
	if (ip_len == 0)
		return (g_buf_alloc(g_buf));
	if (ip_len >= sizeof(startip))
		return NULL;

	/* Wildcards must occupy complete trailing octets (for example 192.168.1.*). */
	j = 0;
	mask = 32;
	wildcard_seen = 0;
	valid = 1;

	for (i = 0; i < (int)ip_len; i++) {
		char c = ip[i];

		if (c == '*') {
			if ((i > 0 && ip[i-1] != '.') ||
			    (i + 1 < (int)ip_len && ip[i+1] != '.')) {
				valid = 0;
				break;
			}
			wildcard_seen = 1;
			mask -= 8;
			startip[j++] = '0';
		} else if (c == '.') {
			startip[j++] = c;
		} else if (c >= '0' && c <= '9' && !wildcard_seen) {
			startip[j++] = c;
		} else {
			valid = 0;
			break;
		}
	}

	startip[j] = '\0';
	if (!valid || mask < 0 || inet_pton(AF_INET, startip, &addr) != 1)
		return NULL;

	if (mask == 32) {
		g_buf_set(startip);
	} else if (mask > 0) {
		char range[20];

		snprintf(range, sizeof(range), "%s/%d", startip, mask);
		g_buf_set(range);
	}

	return (g_buf_alloc(g_buf));
}

#if 0
static int
is_valid_filter_date(const char *nv_date)
{
	char *date = nvram_get(nv_date);
	if (!date)
		date = "1111111";

	if (strlen(date) != 7 || !strcmp(date, "0000000"))
		return 0;

	return 1;
}

static int
is_valid_filter_time(const char *nv_time)
{
	char time_s[8], time_e[8], *time;

	time = nvram_get(nv_time);
	if (!time)
		time = "00002359";

	if (strlen(time) != 8)
		return 0;

	strncpy(time_s, time+0, 4);
	strncpy(time_e, time+4, 4);

	time_s[4] = 0;
	time_e[4] = 0;

	if (atoi(time_s) == atoi(time_e))
		return 0;

	return 1;
}
#endif

void
ip2class(const char *addr, const char *mask, char *out_buf, size_t out_len)
{
	unsigned int val, ip;
	struct in_addr in;
	int i=0;

	// only handle class A,B,C
	val = (unsigned int)inet_addr(mask);
	ip = (unsigned int)inet_addr(addr);

	// oleg patch ~
	in.s_addr = ip & val;

	for (val = ntohl(val); val; i++)
		val <<= 1;

	snprintf(out_buf, out_len, "%s/%d", inet_ntoa(in), i);
}

static int
firewall_read_static_route_entry_count(int *entry_count)
{
	char *value = NULL;
	int api_result, saved_errno, status;

	if (!entry_count) {
		errno = EINVAL;
		return EINVAL;
	}

	errno = 0;
	api_result = nvram_get_checked("sr_num_x", &value);
	saved_errno = errno;
	status = firewall_nvram_int_status(api_result, saved_errno, value, 0,
					   entry_count);
	if (status != 0) {
		errno = status;
		logmessage(LOGNAME,
			   "cannot read static-route entry count (errno=%d)", status);
		return status;
	}
	if (!static_route_entry_count_is_safe(*entry_count)) {
		errno = ERANGE;
		logmessage(LOGNAME,
			   "invalid static-route entry count %d; refusing route update",
			   *entry_count);
		return ERANGE;
	}

	return 0;
}

static int
firewall_copy_static_route_setting(const char *name, int index,
				   char *buffer, size_t buffer_size)
{
	const char *value;
	size_t value_length;
	int status;

	if (!buffer || buffer_size == 0) {
		errno = EINVAL;
		return EINVAL;
	}
	status = firewall_read_indexed_string_setting(name, index, &value);
	if (status != 0)
		return status;

	value_length = strlen(value);
	if (value_length >= buffer_size) {
		errno = ERANGE;
		return ERANGE;
	}
	memcpy(buffer, value, value_length + 1);
	return 0;
}

static int
firewall_copy_static_route_global_setting(const char *name, char *buffer,
					  size_t buffer_size)
{
	const char *value;
	size_t value_length;
	int status;

	if (!name || !name[0] || !buffer || buffer_size == 0) {
		errno = EINVAL;
		return EINVAL;
	}
	status = firewall_read_string_setting(name, &value);
	if (status != 0)
		return status;

	value_length = strlen(value);
	if (value_length >= buffer_size) {
		errno = ERANGE;
		return ERANGE;
	}
	memcpy(buffer, value, value_length + 1);
	return 0;
}

/* WAN, MAN, LAN. Build a complete checked snapshot before the caller mutates routes. */
int
fill_static_routes(char *buf, int len, const char *ift)
{
	int entry_count = 0, i = 0, status, metric_value, written;
	int fallback_gateway_loaded = 0;
	size_t used = 0, remaining;
	char ip[16], netmask[16], gateway[16], metric[16], interface[8];
	char fallback_gateway[16];

	if (!buf || len < 32) {
		errno = EINVAL;
		return EINVAL;
	}
	buf[0] = '\0';
	if (!ift || !ift[0] || !static_route_interface_is_safe(ift)) {
		errno = EINVAL;
		return EINVAL;
	}

	status = firewall_read_static_route_entry_count(&entry_count);
	if (status != 0)
		goto failed;

	for (i = 0; i < entry_count; ++i) {
		status = firewall_copy_static_route_setting("sr_if_x", i,
							    interface,
							    sizeof(interface));
		if (status != 0)
			goto failed;
		if (!static_route_interface_is_safe(interface) ||
		    !interface[0] || strcmp(interface, ift) != 0)
			continue;

		status = firewall_copy_static_route_setting("sr_ipaddr_x", i,
							    ip, sizeof(ip));
		if (status != 0)
			goto failed;
		status = firewall_copy_static_route_setting("sr_netmask_x", i,
							    netmask, sizeof(netmask));
		if (status != 0)
			goto failed;
		status = firewall_copy_static_route_setting("sr_gateway_x", i,
							    gateway, sizeof(gateway));
		if (status != 0)
			goto failed;
		status = firewall_copy_static_route_setting("sr_matric_x", i,
							    metric, sizeof(metric));
		if (status != 0)
			goto failed;

		if (!static_route_ipv4_address_is_valid(ip) ||
		    !static_route_ipv4_address_is_valid(netmask)) {
			status = EINVAL;
			goto failed;
		}
		/* Resolve the legacy WAN gateway fallback with a checked read before
		 * any route mutation. Cache it for all entries in this snapshot. */
		if (!static_route_ipv4_address_is_valid(gateway)) {
			if (!fallback_gateway_loaded) {
				status = firewall_copy_static_route_global_setting(
					"wanx_gateway", fallback_gateway,
					sizeof(fallback_gateway));
				if (status != 0)
					goto failed;
				if (!static_route_ipv4_address_is_valid(fallback_gateway)) {
					status = EINVAL;
					goto failed;
				}
				fallback_gateway_loaded = 1;
			}
			memcpy(gateway, fallback_gateway, strlen(fallback_gateway) + 1);
		}
		status = static_route_metric_parse(metric, &metric_value);
		if (status != 0)
			goto failed;

		if (used != 0) {
			if (used + 1 >= (size_t)len) {
				status = ENOSPC;
				goto failed;
			}
			buf[used++] = ' ';
			buf[used] = '\0';
		}
		remaining = (size_t)len - used;
		written = snprintf(buf + used, remaining, "%s:%s:%s:%d",
				   ip, netmask, gateway, metric_value);
		if (written < 0) {
			status = EIO;
			goto failed;
		}
		if ((size_t)written >= remaining) {
			status = ENOSPC;
			goto failed;
		}
		used += (size_t)written;
	}

	return 0;

failed:
	buf[0] = '\0';
	if (status <= 0)
		status = EIO;
	errno = status;
	logmessage(LOGNAME,
		   "cannot build static-route snapshot at entry %d (errno=%d); refusing route update",
		   i, status);
	return status;
}

static int
include_vpns_clients(FILE *fp, int *lease_lock_fd)
{
	int lock_fd = -1, fd = -1, saved_errno;
	FILE *fpls = NULL;
	struct stat lease_stat;
	char interfaces[MAX_CLIENTS_NUM][VPN_SERVER_LEASE_IFNAME_SIZE];
	size_t interface_count = 0, i;

	if (!fp || !lease_lock_fd) {
		errno = EINVAL;
		return -1;
	}
	*lease_lock_fd = -1;
	lock_fd = vpn_server_lease_lock_acquire(VPN_SERVER_LEASE_LOCK_FILE);
	if (lock_fd < 0)
		goto failed;
	fd = open(VPN_SERVER_LEASE_FILE, O_RDONLY | O_NOFOLLOW);
	if (fd < 0) {
		if (errno != ENOENT)
			goto failed;
	} else {
		if (fstat(fd, &lease_stat) != 0)
			goto failed;
		if (!S_ISREG(lease_stat.st_mode) || lease_stat.st_uid != 0 ||
		    lease_stat.st_nlink != 1 || (lease_stat.st_mode & 077) != 0) {
			errno = EPERM;
			goto failed;
		}
		fpls = fdopen(fd, "r");
		if (!fpls)
			goto failed;
		fd = -1;
		if (vpn_server_lease_read_inventory(fpls, interfaces,
						    MAX_CLIENTS_NUM,
						    &interface_count) != 0)
			goto failed;
		for (i = 0; i < interface_count; ++i) {
			if (!firewall_ppp_interface_is_safe(interfaces[i], VPN_SERVER_PPP_UNIT)) {
				errno = EINVAL;
				goto failed;
			}
		}
		if (fclose(fpls) != 0) {
			fpls = NULL;
			if (errno == 0)
				errno = EIO;
			goto failed;
		}
		fpls = NULL;
	}

	/* Caller retains the lock through iptables-restore, preventing a producer
	 * from changing the validated lease snapshot before its activation. */
	for (i = 0; i < interface_count; ++i) {
		if (fprintf(fp, "-A %s -i %s -j ACCEPT\n",
			    IPT_CHAIN_NAME_VPN_LIST, interfaces[i]) < 0)
			goto failed;
	}
	if (ferror(fp)) {
		errno = EIO;
		goto failed;
	}
	*lease_lock_fd = lock_fd;
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (fpls)
		(void)fclose(fpls);
	if (fd >= 0)
		(void)close(fd);
	if (lock_fd >= 0)
		(void)vpn_server_lease_lock_release(lock_fd);
	logmessage(LOGNAME, "refusing incomplete VPN client firewall inventory (errno=%d)",
		   saved_errno);
	errno = saved_errno;
	return -1;
}


static int
include_mac_filter(FILE *fp, int mac_filter_mode, int num_entries,
		   char *logdrop, int *effective_mode)
{
	int i, mac_num, address_status, schedule_status, status, saved_errno;
	char mac_timematch[128], mac_buf[24];
	const char *mac_value, *ftype;
	const char *dtype = IPT_CHAIN_NAME_MAC_LIST;

	if (!fp || !effective_mode || !firewall_mac_filter_entry_count_is_safe(num_entries)) {
		errno = EINVAL;
		return -1;
	}
	*effective_mode = mac_filter_mode;
	if (mac_filter_mode <= 0)
		return 0;

	if (mac_filter_mode == 2)
		ftype = logdrop;
	else
		ftype = "RETURN";

	mac_num = 0;
	for (i = 0; i < num_entries; ++i) {
		status = firewall_read_indexed_string_setting("macfilter_list_x", i,
							      &mac_value);
		if (status != 0)
			goto failed;

		address_status = firewall_mac_filter_address_format(mac_value,
								     mac_buf,
								     sizeof(mac_buf));
		if (address_status < 0) {
			errno = EINVAL;
			goto failed;
		}
		if (address_status == 0)
			continue;

		/* Preserve legacy accounting: configured rows count even if their
		 * explicit schedule is currently inactive. */
		mac_num++;
		schedule_status = firewall_read_mac_filter_schedule(mac_timematch, i);
		if (schedule_status < 0)
			goto failed;
		if (schedule_status == 0)
			continue;

		if (fprintf(fp, "-A %s -m mac --mac-source %s%s -j %s\n",
			    dtype, mac_buf, mac_timematch, ftype) < 0) {
			if (!errno)
				errno = EIO;
			goto failed;
		}
	}

	if (mac_filter_mode != 2 && mac_num > 0)
		fprintf(fp, "-A %s -j %s\n", dtype, logdrop);

	if (mac_num < 1)
		*effective_mode = 0;

	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	logmessage(LOGNAME,
		   "cannot read or validate MAC-filter entry %d; refusing ruleset publication (errno=%d)",
		   i, saved_errno);
	errno = saved_errno;
	return -1;
}

static int
firewall_url_filter_read_failure(int entry)
{
	int status = errno ? errno : EIO;

	logmessage(LOGNAME,
		   "cannot read URL-filter keyword for entry %d; refusing ruleset publication (errno=%d)",
		   entry, status);
	errno = status;
	return -1;
}

static int
include_webstr_filter(FILE *fp, int num_entries)
{
	int i, webstr_items, url_length, url_total;
	char url_list[256];
	const char *filter_value, *filterstr;
	const char *dtype = IPT_CHAIN_NAME_URL_LIST;
	const char *split = "<&nbsp;>";

	/* webstr support max 256 bytes splitted URL */

	url_total = 0;
	url_list[0] = 0;

	webstr_items = 0;

	for (i = 0; i < num_entries; ++i) {
		if (firewall_read_indexed_string_setting("url_keyword_x", i,
						 &filter_value) != 0)
			return firewall_url_filter_read_failure(i);
		filterstr = filter_value;
		if (strncasecmp(filterstr, "http://", 7) == 0)
			filterstr += 7;
		else if (strncasecmp(filterstr, "https://", 8) == 0)
			filterstr += 8;

		/* Keep untrusted filter values inside the quoted rule syntax. */
		if (!firewall_url_filter_value_is_safe(filterstr))
			continue;

		url_length = strlen(filterstr);
		if (url_length < 1 || url_length >= sizeof(url_list))
			continue;
		
		if (url_total > 0)
			url_length += strlen(split);
		
		if (url_total + url_length < sizeof(url_list)) {
			if (url_total > 0)
				strcat(url_list, split);
		} else {
			url_length = strlen(filterstr);
			
			/* flush merged url */
			fprintf(fp, "-A %s -p tcp -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n",
				dtype, url_list);
			webstr_items++;
			
			url_total = 0;
			url_list[0] = 0;
		}
		strcat(url_list, filterstr);
		url_total += url_length;
	}

	if (url_total) {
		fprintf(fp, "-A %s -p tcp -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n",
			dtype, url_list);
		webstr_items++;
	}

	return webstr_items;
}

static int
firewall_lw_filter_read_failure(const char *field, int entry)
{
	int status = errno ? errno : EIO;

	logmessage(LOGNAME,
		   "cannot read lightweight-filter %s for entry %d; refusing ruleset publication (errno=%d)",
		   field, entry, status);
	errno = status;
	return -1;
}

static int
include_lw_filter(FILE *fp, char *wan_if, char *logaccept, char *logdrop,
			int num_entries, int lw_need_drop)
{
	int i, lw_items;
	char icmp_ptr[64], icmp_list[128], *icmplist, *filterstr, *ftype;
	const char *icmp_value;
	char *proto, *flag, *srcip, *srcport, *dstip, *dstport;
	const char *dtype = IPT_CHAIN_NAME_LWF_LIST;

	if (lw_need_drop)
		ftype = logaccept;
	else
		ftype = logdrop;

	lw_items = 0;

	for (i = 0; i < num_entries; ++i) {
		g_buf_init();
		
		proto = protoflag_conv("filter_lw_proto_x", i, 0);
		if (!proto)
			return firewall_lw_filter_read_failure("protocol", i);
		if (strcmp(proto, "other") == 0) {
			proto = portrange_conv("filter_lw_protono_x", i);
			if (!proto)
				return firewall_lw_filter_read_failure("protocol number", i);
			flag = "";
			srcport = "";
			dstport = "";
			if (!firewall_ip_protocol_number_is_safe(proto))
				continue;
		} else {
			flag = protoflag_conv("filter_lw_proto_x", i, 1);
			if (!flag)
				return firewall_lw_filter_read_failure("TCP flags", i);
			srcport = portrange_conv("filter_lw_srcport_x", i);
			if (!srcport)
				return firewall_lw_filter_read_failure("source port", i);
			dstport = portrange_conv("filter_lw_dstport_x", i);
			if (!dstport)
				return firewall_lw_filter_read_failure("destination port", i);
			if (!firewall_tcp_flags_are_safe(flag) ||
			    !firewall_port_range_is_safe(srcport, 1) ||
			    !firewall_port_range_is_safe(dstport, 1))
				continue;
		}
		
		srcip = iprange_ex_conv("filter_lw_srcip_x", i);
		if (!srcip && errno)
			return firewall_lw_filter_read_failure("source address", i);
		dstip = iprange_ex_conv("filter_lw_dstip_x", i);
		if (!dstip && errno)
			return firewall_lw_filter_read_failure("destination address", i);
		if (!srcip || !dstip) {
			/* Preserve the deny-list's restrictive behavior; whitelist entries
			 * with invalid ranges must never become unrestricted allow rules. */
			if (lw_need_drop)
				continue;
			if (!srcip)
				srcip = "";
			if (!dstip)
				dstip = "";
		}
		
		filterstr = filter_conv(proto, flag, srcip, srcport, dstip, dstport);
		if (*filterstr) {
			fprintf(fp, "-A %s %s -j %s\n", dtype, filterstr, ftype);
			lw_items++;
		}
	}

	if (firewall_read_string_setting("filter_lw_icmp_x", &icmp_value) != 0)
		return firewall_lw_filter_read_failure("ICMP list", -1);
	if (strlen(icmp_value) >= sizeof(icmp_list)) {
		errno = E2BIG;
		return firewall_lw_filter_read_failure("ICMP list length", -1);
	}
	snprintf(icmp_list, sizeof(icmp_list), "%s", icmp_value);
	icmp_ptr[0] = 0;
	foreach(icmp_ptr, icmp_list, icmplist) {
		if (!firewall_icmp_type_is_safe(icmp_ptr))
			continue;

		fprintf(fp, "-A %s -o %s -p icmp --icmp-type %s -j %s\n",
			dtype, wan_if, icmp_ptr, ftype);
		lw_items++;
	}

	if (lw_need_drop)
		fprintf(fp, "-A %s -j %s\n", dtype, logdrop);

	return (lw_items | lw_need_drop);
}

static void
include_vts_filter(FILE *fp, char *lan_ip, char *logaccept, int num_entries, int forward_chain)
{
	int i, proto_status;
	const char *proto;
	char *protono, *port, *lport, *srcip, *dstip, *dtype;
	char srcaddrs[64], dstports[32];

	dtype = (forward_chain) ? "FORWARD" : "INPUT";

	for (i = 0; i < num_entries; ++i)
	{
		g_buf_init();
		srcaddrs[0] = 0;
		
		dstip = ip_conv("vts_ipaddr_x", i);
		if (!firewall_ipv4_address_is_safe(dstip))
			continue;
		
		if (forward_chain) {
			if (strcmp(lan_ip, dstip) == 0)
				continue;
		} else {
			if (strcmp(lan_ip, dstip) != 0)
				continue;
		}
		
		srcip = iprange_ex_conv("vts_srcip_x", i);
		if (!srcip)
			continue;
		if (*srcip)
			snprintf(srcaddrs, sizeof(srcaddrs), " -s %s", srcip);
		
		proto = proto_conv("vts_proto_x", i);
		if (!proto) {
			proto_status = errno ? errno : EIO;
			logmessage(LOGNAME, "skipping virtual-server rule %d: cannot read protocol (errno=%d)", i, proto_status);
			continue;
		}
		port = portrange_conv("vts_port_x", i);
		lport = portrange_conv("vts_lport_x", i);
		if (strcmp(proto, "other") == 0) {
			protono = portrange_conv("vts_protono_x", i);
			if (!firewall_ip_protocol_number_is_safe(protono))
				continue;
		} else {
			if (!firewall_port_range_is_safe(port, 0) ||
			    (lport && *lport && !firewall_port_number_is_safe(lport)))
				continue;
		}
		
		if (lport && strlen(lport)!=0)
			snprintf(dstports, sizeof(dstports), "%s", lport);
		else
			snprintf(dstports, sizeof(dstports), "%s", port);
		
		if (*dstports) {
			if (strcmp(proto, "tcp")==0 || strcmp(proto, "both")==0)
				fprintf(fp, "-A %s -p %s%s -d %s --dport %s -j %s\n", dtype, "tcp", srcaddrs, dstip, dstports, logaccept);
			
			if (strcmp(proto, "udp")==0 || strcmp(proto, "both")==0)
				fprintf(fp, "-A %s -p %s%s -d %s --dport %s -j %s\n", dtype, "udp", srcaddrs, dstip, dstports, logaccept);
		}
		
		if (strcmp(proto, "other")==0) {
			if (*protono)
				fprintf(fp, "-A %s -p %s%s -d %s -j %s\n", dtype, protono, srcaddrs, dstip, logaccept);
		}
	}
}

static void
include_vts_nat(FILE *fp, int num_entries)
{
	int i, proto_status;
	const char *proto;
	char *protono, *port, *lport, *srcip, *dstip, *dtype;
	char srcaddrs[64];

	dtype = IPT_CHAIN_NAME_VSERVER;

	for (i = 0; i < num_entries; ++i)
	{
		g_buf_init();
		srcaddrs[0] = 0;
		
		dstip = ip_conv("vts_ipaddr_x", i);
		if (!firewall_ipv4_address_is_safe(dstip))
			continue;
		
		srcip = iprange_ex_conv("vts_srcip_x", i);
		if (!srcip)
			continue;
		if (*srcip)
			snprintf(srcaddrs, sizeof(srcaddrs), " -s %s", srcip);
		
		proto = proto_conv("vts_proto_x", i);
		if (!proto) {
			proto_status = errno ? errno : EIO;
			logmessage(LOGNAME, "skipping virtual-server rule %d: cannot read protocol (errno=%d)", i, proto_status);
			continue;
		}
		port = portrange_conv("vts_port_x", i);
		lport = portrange_conv("vts_lport_x", i);
		if (strcmp(proto, "other") == 0) {
			protono = portrange_conv("vts_protono_x", i);
			if (!firewall_ip_protocol_number_is_safe(protono))
				continue;
		} else {
			if (!firewall_port_range_is_safe(port, 0) ||
			    (lport && *lport && !firewall_port_number_is_safe(lport)))
				continue;
		}
		
		if (strcmp(proto, "tcp")==0 || strcmp(proto, "both") == 0) {
			if (lport && strlen(lport)!=0 && strcmp(port, lport)!=0)
				fprintf(fp, "-A %s -p %s%s --dport %s -j DNAT --to-destination %s:%s\n", dtype, "tcp", srcaddrs, port, dstip, lport);
			else
				fprintf(fp, "-A %s -p %s%s --dport %s -j DNAT --to %s\n", dtype, "tcp", srcaddrs, port, dstip);
		}
		
		if (strcmp(proto, "udp")==0 || strcmp(proto, "both") == 0) {
			if (lport && strlen(lport)!=0 && strcmp(port, lport)!=0)
				fprintf(fp, "-A %s -p %s%s --dport %s -j DNAT --to-destination %s:%s\n", dtype, "udp", srcaddrs, port, dstip, lport);
			else
				fprintf(fp, "-A %s -p %s%s --dport %s -j DNAT --to %s\n", dtype, "udp", srcaddrs, port, dstip);
		}
		
		if (strcmp(proto, "other") == 0) {
			if (*protono)
				fprintf(fp, "-A %s -p %s%s -j DNAT --to %s\n", dtype, protono, srcaddrs, dstip);
		}
	}
}

static void
include_masquerade(FILE *fp, char *wan_if, char *wan_ip, char *lan_net, int is_fullcone)
{
	char *dtype = "POSTROUTING";

	if (is_fullcone) {
 		fprintf(fp, "-A %s -o %s -s %s -j MASQUERADE --mode fullcone\n", dtype, wan_if, lan_net);
	} else {
		if (wan_ip)
			fprintf(fp, "-A %s -o %s -s %s -j SNAT --to-source %s\n", dtype, wan_if, lan_net, wan_ip);
		else
			fprintf(fp, "-A %s -o %s -s %s -j MASQUERADE\n", dtype, wan_if, lan_net);
	}
}

static int
is_need_tcp_mss_wan(int unit, int wan_proto, char *man_if)
{
	/*if (get_usb_modem_wan(unit) ) {
		int modem_mtu = nvram_safe_get_int("modem_mtu", 1500, 1000, 1500);
		if (modem_mtu != 1500)
			return 1;
	} else {
		if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
		    wan_proto == IPV4_WAN_PROTO_PPTP ||
		    wan_proto == IPV4_WAN_PROTO_L2TP)
			return 1;
		if (get_interface_mtu(man_if) != 1500)
			return 1;
	}*/

	return 1;
}

static char *
get_tcp_mss_ifname_vpns(int vpns_type)
{
#if defined (APP_OPENVPN)
	/* NOTE: Not use TCPMSS target for OpenVPN tunX interface:
	   1) do not use --tun-mtu or --link-mtu (this is changed MTU on tunX interface)
	   2) please use --fragment or/and --mssfix
	*/
	if (vpns_type == 2)
		return NULL;
	else
#endif
		return "ppp+";
}

static char *
get_tcp_mss_ifname_vpnc(int vpnc_type)
{
#if defined (APP_OPENVPN)
	/* NOTE: Not use TCPMSS target for OpenVPN tunX interface:
	   1) do not use --tun-mtu or --link-mtu (this is changed MTU on tunX interface)
	   2) please use --fragment or/and --mssfix
	*/
	if (vpnc_type == 2)
		return NULL;
	else
#endif
		return IFNAME_CLIENT_PPP;
}

static int
get_sshd_bfp_time(int bfp_mode)
{
	int i_bfp_time = 3600;		// 60 min

	if (bfp_mode == 1)
		i_bfp_time = 60;	// 1 min
	else if (bfp_mode == 2)
		i_bfp_time = 300;	// 5 min
	else if (bfp_mode == 3)
		i_bfp_time = 600;	// 10 min
	else if (bfp_mode == 4)
		i_bfp_time = 1800;	// 30 min

	return i_bfp_time;
}

static int
ipt_filter_rules(char *man_if, char *wan_if, char *lan_if, char *lan_ip,
                 char *logaccept, char *logdrop, int tcp_mss_need)
{
	FILE *fp;
	char timematch[160], *ftype, *dtype, *vpnc_if;
	char ipt_file[] = "/tmp/ipt_filter.XXXXXX";
	int ret, lw_status, lw_schedule_status, wport, lport, saved_errno, vpn_lease_lock_fd = -1;
	int is_nat_enabled, is_dos_enabled, is_fw_enabled, is_logaccept, is_logdrop;
	int is_vts_enabled = 0, i_vts_count = 0;
	int is_lw_need_drop = 0, i_lw_count = 0, i_url_count = 0;
	int i_mac_filter_count = 0;
	int is_url_enabled, is_url_inverse, is_lwf_enabled, is_mac_filter_drop;
	int is_ping_enabled, is_udpxy_enabled, is_upnp_enabled = 0;
	int is_sshd_enabled = 1, is_ftp_enabled = 1;
#if defined (APP_XUPNPD)
	int is_xupnpd_enabled;
#endif
	int i_vpns_enable, i_vpns_type, i_vpns_actl, i_http_proto, i_bfplimit_ref;
	int i_bfp_mode = 0;
	int i_vpnc_enable, i_vpnc_type, i_vpnc_sfw, i_mac_filter;
	int is_misc_http_enabled = 0, is_udpxy_wopen = 0;
#if defined (SUPPORT_HTTPS)
	int is_https_wopen = 0;
#endif
#if defined (APP_SSHD)
	int is_sshd_wopen = 0;
#endif
#if defined (APP_FTPD)
	int is_ftpd_wopen = 0;
#endif
	int i_misc_httpport = 0, i_http_lanport = 0;
	int i_udpxy_port = 0, i_udpxy_wport = 0;
#if defined (SUPPORT_HTTPS)
	int i_https_wport = 0, i_https_lport = 0;
#endif
#if defined (APP_SSHD)
	int i_sshd_wport = 0;
#endif
#if defined (APP_FTPD)
	int i_ftpd_wport = 0;
#endif
#if defined (APP_TRMD)
	int is_trmd_supported = 0, is_trmd_enabled = 0, is_trmd_ropen = 0;
	int i_trmd_pport = 0, i_trmd_rport = 0;
#endif
#if defined (APP_ARIA)
	int is_aria_supported = 0, is_aria_enabled = 0, is_aria_ropen = 0;
	int i_aria_pport = 0, i_aria_rport = 0;
#endif
#if defined (APP_OPENVPN)
	int i_vpns_ov_mode = 0, i_vpns_ov_prot = 0, i_vpns_ov_port = 1194;
	int i_vpnc_ov_mode = 0, i_vpnc_ov_cnat = 0;
#endif
#if defined (USE_IPV6)
	int ipv6_type = get_ipv6_type();
#endif

	ret = 0;
	i_bfplimit_ref = 0;

	is_logaccept = (strcmp(logaccept, IPT_CHAIN_NAME_LOG_ACCEPT) == 0) ? 1 : 0;
	is_logdrop = (strcmp(logdrop, IPT_CHAIN_NAME_LOG_DROP) == 0) ? 1 : 0;

	ret = firewall_read_enabled_setting("wan_nat_x", &is_nat_enabled);
	if (ret != 0)
		return ret;
	if (is_nat_enabled) {
		ret = firewall_read_enabled_setting("vts_enable_x", &is_vts_enabled);
		if (ret != 0)
			return ret;
		if (is_vts_enabled) {
			ret = firewall_read_vts_entry_count(&i_vts_count);
			if (ret != 0)
				return ret;
		}
	}
	ret = firewall_read_enabled_setting("fw_dos_x", &is_dos_enabled);
	if (ret != 0)
		return ret;
	ret = firewall_read_enabled_setting("fw_enable_x", &is_fw_enabled);
	if (ret != 0)
		return ret;
	ret = firewall_read_enabled_setting("url_enable_x", &is_url_enabled);
	if (ret != 0)
		return ret;
	if (is_url_enabled) {
		ret = firewall_read_url_entry_count(&i_url_count);
		if (ret != 0)
			return ret;
	}
	ret = firewall_read_enabled_setting("fw_lw_enable_x", &is_lwf_enabled);
	if (ret != 0)
		return ret;
	if (is_lwf_enabled) {
		ret = firewall_read_match_setting("filter_lw_default_x", "DROP", 0,
						  &is_lw_need_drop);
		if (ret != 0)
			return ret;
		ret = firewall_read_lw_entry_count(&i_lw_count);
		if (ret != 0)
			return ret;
	}
	is_url_inverse = 0;
	if (is_url_enabled) {
		ret = firewall_read_enabled_setting("url_inv_x", &is_url_inverse);
		if (ret != 0)
			return ret;
	}

	ret = firewall_read_mac_filter_settings(&i_mac_filter, &is_mac_filter_drop);
	if (ret != 0)
		return ret;

	if (i_mac_filter > 0) {
		ret = firewall_read_mac_filter_entry_count(&i_mac_filter_count);
		if (ret != 0)
			return ret;
	}

	/* Preserve legacy nvram_invmatch(..., "0") defaults, but fail closed
	 * on NVRAM read/close errors before any filter rules are staged. */
	ret = firewall_read_not_zero_setting("misc_ping_x", &is_ping_enabled);
	if (ret != 0)
		return ret;
	ret = firewall_read_not_zero_setting("udpxy_enable_x", &is_udpxy_enabled);
	if (ret != 0)
		return ret;
#if defined (APP_XUPNPD)
	ret = firewall_read_not_zero_setting("xupnpd_enable_x", &is_xupnpd_enabled);
	if (ret != 0)
		return ret;
#endif
#if defined (APP_SSHD)
	ret = firewall_read_not_zero_setting("sshd_enable", &is_sshd_enabled);
	if (ret != 0)
		return ret;
#endif
#if defined (APP_FTPD)
	ret = firewall_read_not_zero_setting("enable_ftp", &is_ftp_enabled);
	if (ret != 0)
		return ret;
#endif
	if (is_nat_enabled) {
		ret = firewall_read_not_zero_setting("upnp_enable_x", &is_upnp_enabled);
		if (ret != 0)
			return ret;
	}

	ret = firewall_read_integer_setting("vpns_enable", 0, &i_vpns_enable);
	if (ret != 0)
		return ret;
	ret = firewall_read_integer_setting("vpns_type", 0, &i_vpns_type);
	if (ret != 0)
		return ret;
	ret = firewall_read_integer_setting("vpns_actl", 0, &i_vpns_actl);
	if (ret != 0)
		return ret;

	ret = firewall_read_integer_setting("vpnc_enable", 0, &i_vpnc_enable);
	if (ret != 0)
		return ret;
	ret = firewall_read_integer_setting("vpnc_type", 0, &i_vpnc_type);
	if (ret != 0)
		return ret;
	ret = firewall_read_integer_setting("vpnc_sfw", 0, &i_vpnc_sfw);
	if (ret != 0)
		return ret;

#if defined (APP_OPENVPN)
	/* Read OpenVPN-only policy only when the corresponding VPN is enabled. */
	if (i_vpns_enable && i_vpns_type == 2) {
		ret = firewall_read_integer_setting("vpns_ov_mode", 0, &i_vpns_ov_mode);
		if (ret != 0)
			return ret;
		ret = firewall_read_integer_setting("vpns_ov_prot", 0, &i_vpns_ov_prot);
		if (ret != 0)
			return ret;
		ret = firewall_read_integer_setting("vpns_ov_port", 1194, &i_vpns_ov_port);
		if (ret != 0)
			return ret;
		if (i_vpns_ov_port < 1 || i_vpns_ov_port > 65535)
			i_vpns_ov_port = 1194;
	}
	if (i_vpnc_enable && i_vpnc_type == 2) {
		ret = firewall_read_integer_setting("vpnc_ov_mode", 0, &i_vpnc_ov_mode);
		if (ret != 0)
			return ret;
		ret = firewall_read_integer_setting("vpnc_ov_cnat", 0, &i_vpnc_ov_cnat);
		if (ret != 0)
			return ret;
	}
#endif

	/* Check remote-listener policy and ports before staging any filter rules. */
	if (is_fw_enabled) {
		i_http_proto = 0;
#if defined (SUPPORT_HTTPS)
		ret = firewall_read_integer_setting("http_proto", 0, &i_http_proto);
		if (ret != 0)
			return ret;
#endif
		if (i_http_proto == 0 || i_http_proto == 2) {
			ret = firewall_read_enabled_setting("misc_http_x", &is_misc_http_enabled);
			if (ret != 0)
				return ret;
			if (is_misc_http_enabled) {
				ret = firewall_read_integer_setting("misc_httpport_x", 0, &i_misc_httpport);
				if (ret != 0)
					return ret;
				ret = firewall_read_integer_setting("http_lanport", 0, &i_http_lanport);
				if (ret != 0)
					return ret;
			}
		}
#if defined (SUPPORT_HTTPS)
		if (i_http_proto == 1 || i_http_proto == 2) {
			ret = firewall_read_enabled_setting("https_wopen", &is_https_wopen);
			if (ret != 0)
				return ret;
			if (is_https_wopen) {
				ret = firewall_read_integer_setting("https_wport", 0, &i_https_wport);
				if (ret != 0)
					return ret;
				ret = firewall_read_integer_setting("https_lport", 0, &i_https_lport);
				if (ret != 0)
					return ret;
			}
		}
#endif
#if defined (APP_SSHD)
		if (is_sshd_enabled) {
			ret = firewall_read_enabled_setting("sshd_wopen", &is_sshd_wopen);
			if (ret != 0)
				return ret;
			if (is_sshd_wopen) {
				ret = firewall_read_integer_setting("sshd_wport", 0, &i_sshd_wport);
				if (ret != 0)
					return ret;
				/* A failed BFP mode read must not silently disable SSH protection. */
				ret = firewall_read_integer_setting("sshd_wbfp", 0, &i_bfp_mode);
				if (ret != 0)
					return ret;
			}
		}
#endif
#if defined (APP_FTPD)
		if (is_ftp_enabled) {
			ret = firewall_read_enabled_setting("ftpd_wopen", &is_ftpd_wopen);
			if (ret != 0)
				return ret;
			if (is_ftpd_wopen) {
				ret = firewall_read_integer_setting("ftpd_wport", 0, &i_ftpd_wport);
				if (ret != 0)
					return ret;
			}
		}
#endif
		ret = firewall_read_enabled_setting("udpxy_wopen", &is_udpxy_wopen);
		if (ret != 0)
			return ret;
		if (is_udpxy_wopen) {
			ret = firewall_read_integer_setting("udpxy_enable_x", 0, &i_udpxy_port);
			if (ret != 0)
				return ret;
			if (firewall_unprivileged_port_is_safe(i_udpxy_port)) {
				ret = firewall_read_integer_setting("udpxy_wport", 0, &i_udpxy_wport);
				if (ret != 0)
					return ret;
			}
		}
#if defined (APP_TRMD)
		is_trmd_supported = is_torrent_support();
		if (is_trmd_supported) {
			ret = firewall_read_enabled_setting("trmd_enable", &is_trmd_enabled);
			if (ret != 0)
				return ret;
			if (is_trmd_enabled) {
				ret = firewall_read_integer_setting("trmd_pport", 0, &i_trmd_pport);
				if (ret != 0)
					return ret;
				if (i_trmd_pport < 1024 || i_trmd_pport > 65535)
					i_trmd_pport = TRANSMISSION_PPORT;
				ret = firewall_read_enabled_setting("trmd_ropen", &is_trmd_ropen);
				if (ret != 0)
					return ret;
				if (is_trmd_ropen) {
					ret = firewall_read_integer_setting("trmd_rport", 0, &i_trmd_rport);
					if (ret != 0)
						return ret;
					if (i_trmd_rport < 1024 || i_trmd_rport > 65535)
						i_trmd_rport = TRANSMISSION_RPORT;
				}
			}
		}
#endif
#if defined (APP_ARIA)
		is_aria_supported = is_aria_support();
		if (is_aria_supported) {
			ret = firewall_read_enabled_setting("aria_enable", &is_aria_enabled);
			if (ret != 0)
				return ret;
			if (is_aria_enabled) {
				ret = firewall_read_integer_setting("aria_pport", 0, &i_aria_pport);
				if (ret != 0)
					return ret;
				if (i_aria_pport < 1024 || i_aria_pport > 65535)
					i_aria_pport = ARIA_PPORT;
				ret = firewall_read_enabled_setting("aria_ropen", &is_aria_ropen);
				if (ret != 0)
					return ret;
				if (is_aria_ropen) {
					ret = firewall_read_integer_setting("aria_rport", 0, &i_aria_rport);
					if (ret != 0)
						return ret;
					if (i_aria_rport < 1024 || i_aria_rport > 65535)
						i_aria_rport = ARIA_RPORT;
				}
			}
		}
#endif
	}
	vpnc_if = NULL;
	if (i_vpnc_enable) {
#if defined (APP_OPENVPN)
		if (i_vpnc_type == 2) {
			if (i_vpnc_ov_mode == 1)
				vpnc_if = IFNAME_CLIENT_TUN;
			else if (i_vpnc_ov_cnat == 1)
				vpnc_if = IFNAME_CLIENT_TAP;
		} else
#endif
			vpnc_if = IFNAME_CLIENT_PPP;
	}

	if (!(fp=firewall_open_rule_file(ipt_file, "IPv4 filter")))
		return 0;

	fprintf(fp, "*%s\n", "filter");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", (is_fw_enabled) ? "DROP" :  "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_FORWARD);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_VPN_LIST);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_BFP_LIMIT);
	if (i_mac_filter > 0)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_MAC_LIST);
	if (is_url_enabled)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_URL_LIST);
	if (is_lwf_enabled)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LWF_LIST);
	if (is_dos_enabled)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_DOS_LIMIT);
	if (is_logaccept)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LOG_ACCEPT);
	if (is_logdrop)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LOG_DROP);

	// maclist chain
	if (include_mac_filter(fp, i_mac_filter, i_mac_filter_count,
				    logdrop, &i_mac_filter) != 0) {
		saved_errno = errno ? errno : EIO;
		(void)fclose(fp);
		firewall_abort_core_restore_after_staging_failure();
		firewall_rule_file_remove(ipt_file);
		errno = saved_errno;
		return -1;
	}

	// INPUT chain
	dtype = "INPUT";

	/* Policy for all traffic from MAC-filtered LAN clients */
	if (i_mac_filter > 0 && is_mac_filter_drop)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_MAC_LIST);

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", "ACCEPT");

	if (is_fw_enabled) {
		/* Accept all traffic from LAN clients */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");
		
		/* Accept all traffic from localhost */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, "lo", "ACCEPT");
		
#if defined (USE_IPV6)
		/* Accept SIT (IPv6-in-IPv4) packets (might be seen as INVALID) */
		if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
			fprintf(fp, "-A %s -p %d -j %s\n", dtype, 41, "ACCEPT");
#endif
		/* Drop the wrong state, INVALID, packets */
		fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", logdrop);
		
		/* Accept multicast (all, except udp port 1900) */
		if (nvram_match("mr_enable_x", "1") || is_udpxy_enabled
#if defined (APP_XUPNPD)
		 || is_xupnpd_enabled
#endif
		) {
			fprintf(fp, "-A %s -p 2 -d 224.0.0.0/4 -j %s\n", dtype, logaccept);
			fprintf(fp, "-A %s -p udp -d 224.0.0.0/4 ! --dport %d -j %s\n", dtype, 1900, "ACCEPT");
		}
	}

	/* DoS attack limits */
	if (is_dos_enabled)
		fprintf(fp, "-A %s -i %s -m %s %s -j %s\n", dtype, wan_if, CT_STATE, "NEW", IPT_CHAIN_NAME_DOS_LIMIT);

	if (is_fw_enabled) {
		/* Block or Accept inbound traffic from VPN client */
		if (vpnc_if) {
			if (i_vpnc_sfw == 1)
				fprintf(fp, "-A %s -i %s ! -p icmp -j %s\n", dtype, vpnc_if, "DROP");
			else if (i_vpnc_sfw == 0 || i_vpnc_sfw == 2)
				fprintf(fp, "-A %s -i %s -j %s\n", dtype, vpnc_if, "ACCEPT");
		}
		
		/* Accept DHCPv4 */
		fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 67, 68, logaccept);
		
		if ((i_http_proto == 0 || i_http_proto == 2) && is_misc_http_enabled) {
			wport = i_misc_httpport;
			lport = i_http_lanport;
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, logaccept);
		}
#if defined (SUPPORT_HTTPS)
		if ((i_http_proto == 1 || i_http_proto == 2) && is_https_wopen) {
			wport = i_https_wport;
			lport = i_https_lport;
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, logaccept);
		}
#endif
#if defined (APP_SSHD)
		if (is_sshd_enabled && is_sshd_wopen) {
			i_bfplimit_ref++;
			wport = i_sshd_wport;
			lport = 22;
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, IPT_CHAIN_NAME_BFP_LIMIT);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, IPT_CHAIN_NAME_BFP_LIMIT);
		}
#endif
#if defined (APP_FTPD)
		if (is_ftp_enabled && is_ftpd_wopen) {
			wport = i_ftpd_wport;
			lport = 21;
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, logaccept);
		}
#endif
		lport = i_udpxy_port;
		if (firewall_unprivileged_port_is_safe(lport) && is_udpxy_wopen) {
			wport = i_udpxy_wport;
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, logaccept);
		}
#if defined (APP_TRMD)
		if (is_trmd_enabled && is_trmd_supported) {
			wport = i_trmd_pport;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (is_trmd_ropen) {
				wport = i_trmd_rport;
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			}
		}
#endif
#if defined (APP_ARIA)
		if (is_aria_enabled && is_aria_supported) {
			wport = i_aria_pport;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (is_aria_ropen) {
				wport = i_aria_rport;
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			}
		}
#endif
		if (i_vpns_enable) {
			int i_need_vpnlist = 1;
#if defined (APP_OPENVPN)
			if (i_vpns_type == 2) {
				const char *ov_prot = "udp";
				
				if (i_vpns_ov_mode == 0)
					i_need_vpnlist = 0;
				if (i_vpns_ov_prot == 1 || i_vpns_ov_prot == 3 || i_vpns_ov_prot == 5)
					ov_prot = "tcp";
				if (i_vpns_ov_prot == 0 || i_vpns_ov_prot == 1 || i_vpns_ov_prot == 4 || i_vpns_ov_prot == 5)
					fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, ov_prot, i_vpns_ov_port, logaccept);
			} else
#endif
			if (i_vpns_type == 1) {
				/* accept L2TP */
				fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, "udp", 1701, logaccept);
			} else {
				/* accept PPTP */
				fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, "tcp", 1723, logaccept);
				fprintf(fp, "-A %s -p %d -j %s\n", dtype, 47, logaccept);
			}
			
			/* Jump to vpnlist chain */
			if (i_need_vpnlist) {
				if (i_vpns_actl == 0 || i_vpns_actl == 2) {
					/* accept all packets */
					fprintf(fp, "-A %s -j %s\n", dtype, IPT_CHAIN_NAME_VPN_LIST);
				} else {
					/* accept DNS UDP packets only */
					fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, "udp", 53, IPT_CHAIN_NAME_VPN_LIST);
				}
			}
		}
		
		/* add Virtual Server rules for router host */
		if (is_nat_enabled && is_vts_enabled)
			include_vts_filter(fp, lan_ip, logaccept, i_vts_count, 0);
		
		/* Accept ICMPv4 packets */
		if (is_ping_enabled) {
			/* Accept all ICMP and UDP ports 33434-33534 for traceroute */
			fprintf(fp, "-A %s -p icmp -j %s\n", dtype, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d:%d -j %s\n", dtype, 33434, 33534, logaccept);
		} else {
			/* Accept all ICMP, exclude ping */
			fprintf(fp, "-A %s -p icmp ! --icmp-type %s -j %s\n", dtype, "echo-request", logaccept);
#if defined (USE_IPV6)
			if (ipv6_type == IPV6_6IN4) {
				/* pass ICMP ping from 6in4 remote endpoint */
				char *tun_remote = nvram_safe_get("ip6_6in4_remote");
				if (firewall_ipv4_address_is_safe(tun_remote))
					fprintf(fp, "-A %s -p icmp --icmp-type %s -s %s -j %s\n", dtype, "echo-request", tun_remote, logaccept);
			}
#endif
		}
		
		/* Drop all (only for log) */
		if (is_logdrop)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	// FORWARD chain
	dtype = "FORWARD";

	if (i_mac_filter > 0)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_MAC_LIST);

	if (is_fw_enabled) {
		/* Accept the redirect packets (might be seen as INVALID) */
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, lan_if, lan_if, "ACCEPT");
	}

	/* use url filter before accepting ESTABLISHED packets */
	if (is_url_enabled) {
		int url_status = include_webstr_filter(fp, i_url_count);
		if (url_status < 0) {
			saved_errno = errno ? errno : EIO;
			logmessage(LOGNAME,
				   "IPv4 filter: aborting incomplete URL-filter ruleset (errno=%d)",
				   saved_errno);
			(void)fclose(fp);
			firewall_abort_core_restore_after_staging_failure();
			firewall_rule_file_remove(ipt_file);
			errno = saved_errno;
			return -1;
		}
		if (url_status > 0) {
		char mac_buf[24] = {0};
		timematch[0] = 0;
		if (timematch_conv(timematch, "url_date_x", "url_time_x")) {
			mac_conv("url_mac_x", -1, mac_buf);
			if (strlen(mac_buf) == 17) {
				strcat(timematch, " -m mac");
				if (is_url_inverse)
					strcat(timematch, " !");
				strcat(timematch, " --mac-source ");
				strcat(timematch, mac_buf);
			}
			fprintf(fp, "-A %s -i %s%s -j %s\n", dtype, lan_if, timematch, IPT_CHAIN_NAME_URL_LIST);
			ret |= MODULE_WEBSTR_MASK;
		}
		}
	}

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
	if (!tcp_mss_need) {
		if (i_vpnc_enable) {
			char *vpnc_if = get_tcp_mss_ifname_vpnc(i_vpnc_type);
			if (vpnc_if)
				fprintf(fp, "-A %s%s -o %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype, "", vpnc_if);
		}
		if (i_vpns_enable) {
			char *vpns_if = get_tcp_mss_ifname_vpns(i_vpns_type);
			if (vpns_if)
				fprintf(fp, "-A %s%s -i %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype, "", vpns_if);
		}
	} else {
		fprintf(fp, "-A %s%s -o %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype, " !", lan_if);
	}

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", "ACCEPT");

	if (is_fw_enabled) {
		/* Drop all packets in the INVALID state */
		fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", logdrop);
		
		/* Pass multicast (do not log) */
		if (nvram_match("mr_enable_x", "1"))
			fprintf(fp, "-A %s -p udp -d 224.0.0.0/4 -j %s\n", dtype, "ACCEPT");
#if 0
		/* Filter out invalid WAN->WAN connections */
		fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, wan_if, lan_if, logdrop);
		if (strcmp(man_if, wan_if))
			fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, man_if, lan_if, logdrop);
#endif
	}

	/* DoS attacks from WAN */
	if (is_dos_enabled)
		fprintf(fp, "-A %s -i %s -m %s %s -j %s\n", dtype, wan_if, CT_STATE, "NEW", IPT_CHAIN_NAME_DOS_LIMIT);

	/* FILTER from LAN to WAN */
	if (is_lwf_enabled) {
		lw_schedule_status = timematch_lw_filter_conv(timematch);
		if (lw_schedule_status < 0) {
			saved_errno = errno ? errno : EIO;
			logmessage(LOGNAME,
				   "IPv4 filter: aborting after Lightweight Filter schedule read error (errno=%d)",
				   saved_errno);
			(void)fclose(fp);
			firewall_abort_core_restore_after_staging_failure();
			firewall_rule_file_remove(ipt_file);
			errno = saved_errno;
			return -1;
		}
		if (lw_schedule_status > 0) {
			lw_status = include_lw_filter(fp, wan_if, logaccept, logdrop,
					      i_lw_count, is_lw_need_drop);
			if (lw_status < 0) {
				saved_errno = errno ? errno : EIO;
				logmessage(LOGNAME,
					   "IPv4 filter: aborting incomplete Lightweight Filter ruleset (errno=%d)",
					   saved_errno);
				(void)fclose(fp);
				firewall_abort_core_restore_after_staging_failure();
				firewall_rule_file_remove(ipt_file);
				errno = saved_errno;
				return -1;
			}
			if (lw_status > 0)
				fprintf(fp, "-A %s -i %s%s -j %s\n",
					dtype, lan_if, timematch, IPT_CHAIN_NAME_LWF_LIST);
		}
	}


	if (is_fw_enabled) {
		/* Accept LAN other outbound traffic */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");
		
		/* Pass ICMPv4 in gateway mode w/o NAT */
		if (!is_nat_enabled) {
			if (is_ping_enabled)
				fprintf(fp, "-A %s -p icmp -j %s\n", dtype, logaccept);
			else
				fprintf(fp, "-A %s -p icmp ! --icmp-type %s -j %s\n", dtype, "echo-request", logaccept);
		}
		
		/* Accept VPN server client's outbound traffic */
		if (i_vpns_enable) {
			int i_need_vpnlist = 1;
#if defined (APP_OPENVPN)
			if (i_vpns_type == 2 && i_vpns_ov_mode == 0)
				i_need_vpnlist = 0;
#endif
			/* Jump to vpnlist chain */
			if (i_need_vpnlist) {
				if (i_vpns_actl == 0 || i_vpns_actl == 1)
					fprintf(fp, "-A %s -j %s\n", dtype, IPT_CHAIN_NAME_VPN_LIST);
				else if (i_vpns_actl == 4)
					fprintf(fp, "-A %s -o %s -j %s\n", dtype, wan_if, IPT_CHAIN_NAME_VPN_LIST);
				else
					fprintf(fp, "-A %s -o %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_VPN_LIST);
			}
		}
		
		/* Block or Accept inbound traffic from VPN client (do not log) */
		if (vpnc_if) {
			if (i_vpnc_sfw == 1)
				fprintf(fp, "-A %s -i %s -j %s\n", dtype, vpnc_if, "DROP");
			else if (i_vpnc_sfw == 0 || i_vpnc_sfw == 2)
				fprintf(fp, "-A %s -i %s -j %s\n", dtype, vpnc_if, "ACCEPT");
		}
		
		if (is_nat_enabled) {
			char *dmz_ip;
			
			/* Accept to BattleNET */
			if (nvram_match("sp_battle_ips", "1"))
				fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, BATTLENET_PORT, logaccept);
			
#if defined (USE_MATCH_CONNTRACK)
			/* Accept to Virtual Servers, UPnP, DMZ */
			fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, "conntrack --ctstate", "DNAT", logaccept);
#else
			/* Accept to exposed station (DMZ) */
			dmz_ip = nvram_safe_get("dmz_ip");
			if (firewall_ipv4_address_is_safe(dmz_ip))
				fprintf(fp, "-A %s -d %s -j %s\n", dtype, dmz_ip, logaccept);
			
			/* Accept to Virtual Servers */
			if (is_vts_enabled)
				include_vts_filter(fp, lan_ip, logaccept, i_vts_count, 1);
			
			/* Jump to IGD UPnP/NAT-PMP (miniupnpd chain) */
			if (is_upnp_enabled)
				fprintf(fp, "-A %s -j %s\n", dtype, MINIUPNPD_CHAIN_IP4_FORWARD);
#endif
		}
		
		/* Drop all (only for log) */
		if (is_logdrop)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	/* vpnlist chain */
	if (i_vpns_enable) {
		dtype = IPT_CHAIN_NAME_VPN_LIST;
		ftype = "ACCEPT";
#if defined (APP_OPENVPN)
		if (i_vpns_type == 2 && i_vpns_ov_mode == 1)
			fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_SERVER_TUN, ftype);
		else
#endif
			if (include_vpns_clients(fp, &vpn_lease_lock_fd) != 0) {
				saved_errno = errno ? errno : EIO;
				logmessage(LOGNAME,
					   "IPv4 filter: refusing VPN_LIST rules from incomplete lease inventory (errno=%d)",
					   saved_errno);
				(void)fclose(fp);
				firewall_abort_core_restore_after_staging_failure();
				firewall_rule_file_remove(ipt_file);
				errno = saved_errno;
				return -1;
			}
	}

	/* bfplimit chain */
	dtype = IPT_CHAIN_NAME_BFP_LIMIT;
	if (i_bfplimit_ref) {
		if (i_bfp_mode > 0) {
			fprintf(fp, "-A %s -m recent --set --name %s\n", dtype, "blacklist");
			fprintf(fp, "-A %s -m recent --update --hitcount %d --seconds %d --name %s -j %s\n", dtype, 4, get_sshd_bfp_time(i_bfp_mode), "blacklist", logdrop);
		}
		fprintf(fp, "-A %s -j %s\n", dtype, logaccept);
	}

	/* doslimit chain */
	if (is_dos_enabled) {
		dtype = IPT_CHAIN_NAME_DOS_LIMIT;
		ftype = "RETURN";
		fprintf(fp, "-A %s -p tcp --syn -m limit --limit 20/s --limit-burst 30 -j %s\n", dtype, ftype);
		fprintf(fp, "-A %s -p tcp --syn -j %s\n", dtype, logdrop);
		fprintf(fp, "-A %s -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j %s\n", dtype, ftype);
		fprintf(fp, "-A %s -p tcp --tcp-flags SYN,ACK,FIN,RST RST -j %s\n", dtype, logdrop);
		fprintf(fp, "-A %s -p icmp --icmp-type %s -m limit --limit 2/s -j %s\n", dtype, "echo-request", ftype);
		fprintf(fp, "-A %s -p icmp --icmp-type %s -j %s\n", dtype, "echo-request", logdrop);
	}

	/* logaccept chain */
	if (is_logaccept) {
		dtype = IPT_CHAIN_NAME_LOG_ACCEPT;
		ftype = "ACCEPT";
		fprintf(fp, "-A %s -m %s %s -j LOG --log-prefix \"%s \" "
			    "--log-tcp-options --log-ip-options\n", dtype, CT_STATE, "NEW", ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, ftype);
	}

	/* logdrop chain */
	if (is_logdrop) {
		dtype = IPT_CHAIN_NAME_LOG_DROP;
		ftype = "DROP";
		fprintf(fp, "-A %s -m %s %s -j LOG --log-prefix \"%s \" "
			    "--log-tcp-options --log-ip-options\n", dtype, CT_STATE, "NEW,INVALID", ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, ftype);
	}

	fprintf(fp, "COMMIT\n\n");
	if (firewall_finish_rule_file(fp, ipt_file, "IPv4 filter") == 0) {
		if (ret & MODULE_WEBSTR_MASK)
			module_smart_load("xt_webstr", NULL);
		firewall_apply_core_restore("IPv4 filter restore", "iptables", "filter", ipt_file);
	}
	firewall_rule_file_remove(ipt_file);
	if (vpn_lease_lock_fd >= 0 &&
	    vpn_server_lease_lock_release(vpn_lease_lock_fd) != 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME, "cannot release VPN lease lock after firewall restore (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return -1;
	}

	return ret;
}

int
ipt_filter_default(void)
{
	FILE *fp;
	char *ftype, *dtype;
	int is_fw_enabled, result = -1, firewall_status;
	char ipt_file[] = "/tmp/ipt_filter.XXXXXX";

	firewall_status = firewall_read_enabled_setting("fw_enable_x", &is_fw_enabled);
	if (firewall_status != 0)
		return firewall_status;

	if (!(fp=firewall_open_rule_file(ipt_file, "IPv4 default filter")))
		return -1;

	fprintf(fp, "*%s\n", "filter");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_FORWARD);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_VPN_LIST);

	/* INPUT chain */
	dtype = "INPUT";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, "lo", ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", "DROP");
	if (is_fw_enabled)
		fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 67, 68, ftype);

	/* FORWARD chain */
	dtype = "FORWARD";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", ftype);
	fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, IFNAME_BR, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", "DROP");
	if (is_fw_enabled)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);

	fprintf(fp, "COMMIT\n\n");
	if (firewall_finish_rule_file(fp, ipt_file, "IPv4 default filter") == 0)
		result = firewall_apply_core_restore("IPv4 default filter restore", "iptables", "filter", ipt_file);
	firewall_rule_file_remove(ipt_file);
	return result;
}

static int
firewall_module_table_restore_required(const char *operation,
				       const char *module_name,
				       int force_restore,
				       int *should_restore)
{
	int probe_status, status;

	probe_status = is_module_loaded_status(module_name);
	status = rc_firewall_module_table_restore_decide(probe_status,
						 force_restore, should_restore);
	if (status != 0) {
		logmessage(LOGNAME,
			   "%s: cannot verify %s module state; aborting firewall rebuild (status=%d)",
			   operation, module_name, status);
		firewall_core_restore_transaction.failed = 1;
		errno = status;
	}
	return status;
}

static void
ipt_mangle_rules(const char *man_if, const char *wan_if, int use_man)
{
	FILE *fp;
	int i_wan_ttl_fix;
	int i_wan_ttl_value;
	int is_multicast_enabled = 0;
	int status, should_restore;
	const char *dtype;
	char ipt_file[] = "/tmp/ipt_mangle.XXXXXX";

	/* These NVRAM reads control packet TTL policy and must not turn backend
	 * failures into a silent mode-0 ruleset. Finish the checked snapshot before
	 * touching sysctls, loading modules or staging the mangle table. */
	status = firewall_read_integer_setting("wan_ttl_fix", 0, &i_wan_ttl_fix);
	if (status != 0)
		return;
	status = firewall_read_integer_setting("wan_ttl_value", 0, &i_wan_ttl_value);
	if (status != 0)
		return;

	if (i_wan_ttl_fix == 2) {
		status = firewall_read_match_setting("mr_enable_x", "1", 0,
					     &is_multicast_enabled);
		if (status != 0)
			return;
		if (!is_multicast_enabled)
			i_wan_ttl_fix = 0;
	}

	if (i_wan_ttl_value > 1) {
		int write_status, write_errno, ttl_status;

		write_status = fput_int("/proc/sys/net/ipv4/ip_default_ttl", i_wan_ttl_value);
		write_errno = errno;
		ttl_status = firewall_mangle_ttl_write_status(
			i_wan_ttl_value, write_status, write_errno);
		if (ttl_status != 0) {
			logmessage(LOGNAME,
				   "IPv4 default TTL update failed for configured value %d (status=%d); aborting firewall rebuild",
				   i_wan_ttl_value, ttl_status);
			firewall_core_restore_transaction.failed = 1;
			errno = ttl_status;
			return;
		}
	}

	if (i_wan_ttl_fix) {
		module_smart_load("iptable_mangle", NULL);
		module_smart_load("xt_HL", NULL);
	}
	if (firewall_module_table_restore_required("IPv4 mangle restore",
						   "iptable_mangle", i_wan_ttl_fix,
						   &should_restore) != 0 || !should_restore)
		return;

	if (!(fp=firewall_open_rule_file(ipt_file, "IPv4 mangle")))
		return;

	fprintf(fp, "*%s\n", "mangle");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");

	dtype = "PREROUTING";

	if (i_wan_ttl_fix) {
		const char *viptv_if = man_if;
		const char *viptv_iflast = nvram_safe_get("viptv_ifname");
		if (*viptv_iflast && is_interface_exist(viptv_iflast))
			viptv_if = viptv_iflast;
		
		if (i_wan_ttl_fix == 1) {
			fprintf(fp, "-A %s -i %s -j TTL %s %d\n", dtype, wan_if, "--ttl-inc", 1);
			if (use_man)
				fprintf(fp, "-A %s -i %s -j TTL %s %d\n", dtype, man_if, "--ttl-inc", 1);
		}
		
		if (i_wan_ttl_fix == 2 || (strcmp(wan_if, viptv_if) && strcmp(man_if, viptv_if)))
			fprintf(fp, "-A %s -i %s -p udp -d 224.0.0.0/4 -j TTL %s %d\n", dtype, viptv_if, "--ttl-inc", 1);
	}

	fprintf(fp, "COMMIT\n\n");
	if (firewall_finish_rule_file(fp, ipt_file, "IPv4 mangle") == 0 && should_restore)
		firewall_apply_core_restore("IPv4 mangle restore", "iptables", "mangle", ipt_file);
	firewall_rule_file_remove(ipt_file);
}

static void
ipt_raw_rules(void)
{
	FILE *fp;
	int should_restore;
	char ipt_file[] = "/tmp/ipt_raw.XXXXXX";

	if (firewall_module_table_restore_required("IPv4 raw restore",
						   "iptable_raw", 0,
						   &should_restore) != 0 || !should_restore)
		return;
	if (!(fp=firewall_open_rule_file(ipt_file, "IPv4 raw")))
		return;

	fprintf(fp, "*%s\n", "raw");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");

	fprintf(fp, "COMMIT\n\n");
	if (firewall_finish_rule_file(fp, ipt_file, "IPv4 raw") == 0 && should_restore)
		firewall_apply_core_restore("IPv4 raw restore", "iptables", "raw", ipt_file);
	firewall_rule_file_remove(ipt_file);
}

#if defined (USE_IPV6)
static int
ip6t_filter_rules(char *man_if, char *wan_if, char *lan_if,
                  char *logaccept, char *logdrop, int tcp_mss_need)
{
	FILE *fp;
	char timematch[160], *ftype, *dtype;
	int ret, saved_errno, wport, lport;
	int ipv6_type, is_fw_enabled, is_url_enabled, is_url_inverse, is_logaccept, is_logdrop;
	int i_url_count = 0, i_mac_filter_count = 0;
	int i_http_proto, i_bfplimit_ref, i_mac_filter, is_mac_filter_drop;
	int i_bfp_mode = 0;
	int is_ping_enabled = 0, is_upnp_enabled = 0;
	int is_vpns_enabled = 0, i_vpns_type = 0;
	int is_misc_http_enabled = 0, is_udpxy_wopen = 0;
#if defined (SUPPORT_HTTPS)
	int is_https_wopen = 0;
#endif
#if defined (APP_SSHD)
	int is_sshd_wopen = 0;
#endif
#if defined (APP_FTPD)
	int is_ftpd_wopen = 0;
#endif
	int i_misc_httpport = 0, i_http_lanport = 0;
	int i_udpxy_port = 0, i_udpxy_wport = 0;
#if defined (SUPPORT_HTTPS)
	int i_https_wport = 0, i_https_lport = 0;
#endif
#if defined (APP_SSHD)
	int i_sshd_wport = 0;
#endif
#if defined (APP_FTPD)
	int i_ftpd_wport = 0;
#endif
#if defined (APP_TRMD)
	int is_trmd_supported = 0, is_trmd_enabled = 0, is_trmd_ropen = 0;
	int i_trmd_pport = 0, i_trmd_rport = 0;
#endif
#if defined (APP_ARIA)
	int is_aria_supported = 0, is_aria_enabled = 0, is_aria_ropen = 0;
	int i_aria_pport = 0, i_aria_rport = 0;
#endif
#if defined (APP_OPENVPN)
	int i_vpns_ov_prot = 0, i_vpns_ov_port = 1194;
#endif
#if defined (APP_SSHD)
	int is_sshd_enabled = 1;
#endif
#if defined (APP_FTPD)
	int is_ftp_enabled = 1;
#endif
	char ipt_file[] = "/tmp/ip6t_filter.XXXXXX";

	ret = 0;
	i_bfplimit_ref = 0;

	ipv6_type = get_ipv6_type();

	ret = firewall_read_enabled_setting("fw_enable_x", &is_fw_enabled);
	if (ret != 0)
		return ret;
	ret = firewall_read_enabled_setting("url_enable_x", &is_url_enabled);
	if (ret != 0)
		return ret;
	if (is_url_enabled) {
		ret = firewall_read_url_entry_count(&i_url_count);
		if (ret != 0)
			return ret;
	}
	is_url_inverse = 0;
	if (is_url_enabled) {
		ret = firewall_read_enabled_setting("url_inv_x", &is_url_inverse);
		if (ret != 0)
			return ret;
	}

	ret = firewall_read_mac_filter_settings(&i_mac_filter, &is_mac_filter_drop);
	if (ret != 0)
		return ret;

	if (i_mac_filter > 0) {
		ret = firewall_read_mac_filter_entry_count(&i_mac_filter_count);
		if (ret != 0)
			return ret;
	}

	if (is_fw_enabled) {
		ret = firewall_read_enabled_setting("vpns_enable", &is_vpns_enabled);
		if (ret != 0)
			return ret;
		if (is_vpns_enabled) {
			ret = firewall_read_integer_setting("vpns_type", 0, &i_vpns_type);
			if (ret != 0)
				return ret;
#if defined (APP_OPENVPN)
			if (i_vpns_type == 2) {
				ret = firewall_read_integer_setting("vpns_ov_prot", 0, &i_vpns_ov_prot);
				if (ret != 0)
					return ret;
				ret = firewall_read_integer_setting("vpns_ov_port", 1194, &i_vpns_ov_port);
				if (ret != 0)
					return ret;
				if (i_vpns_ov_port < 1 || i_vpns_ov_port > 65535)
					i_vpns_ov_port = 1194;
			}
#endif
		}
	}

	/* Resolve inverted IPv6 filter selectors before opening the rule file. */
	if (is_fw_enabled) {
		ret = firewall_read_not_zero_setting("misc_ping_x", &is_ping_enabled);
		if (ret != 0)
			return ret;
#if defined (APP_SSHD)
		ret = firewall_read_not_zero_setting("sshd_enable", &is_sshd_enabled);
		if (ret != 0)
			return ret;
#endif
#if defined (APP_FTPD)
		ret = firewall_read_not_zero_setting("enable_ftp", &is_ftp_enabled);
		if (ret != 0)
			return ret;
#endif
		ret = firewall_read_not_zero_setting("upnp_enable_x", &is_upnp_enabled);
		if (ret != 0)
			return ret;
	}

	/* Check remote-listener policy and ports before staging any IPv6 rules. */
	if (is_fw_enabled) {
		i_http_proto = 0;
#if defined (SUPPORT_HTTPS)
		ret = firewall_read_integer_setting("http_proto", 0, &i_http_proto);
		if (ret != 0)
			return ret;
#endif
		if (i_http_proto == 0 || i_http_proto == 2) {
			ret = firewall_read_enabled_setting("misc_http_x", &is_misc_http_enabled);
			if (ret != 0)
				return ret;
			if (is_misc_http_enabled) {
				ret = firewall_read_integer_setting("misc_httpport_x", 0, &i_misc_httpport);
				if (ret != 0)
					return ret;
				ret = firewall_read_integer_setting("http_lanport", 0, &i_http_lanport);
				if (ret != 0)
					return ret;
			}
		}
#if defined (SUPPORT_HTTPS)
		if (i_http_proto == 1 || i_http_proto == 2) {
			ret = firewall_read_enabled_setting("https_wopen", &is_https_wopen);
			if (ret != 0)
				return ret;
			if (is_https_wopen) {
				ret = firewall_read_integer_setting("https_wport", 0, &i_https_wport);
				if (ret != 0)
					return ret;
				ret = firewall_read_integer_setting("https_lport", 0, &i_https_lport);
				if (ret != 0)
					return ret;
			}
		}
#endif
#if defined (APP_SSHD)
		if (is_sshd_enabled) {
			ret = firewall_read_enabled_setting("sshd_wopen", &is_sshd_wopen);
			if (ret != 0)
				return ret;
			if (is_sshd_wopen) {
				ret = firewall_read_integer_setting("sshd_wport", 0, &i_sshd_wport);
				if (ret != 0)
					return ret;
				/* A failed BFP mode read must not silently disable SSH protection. */
				ret = firewall_read_integer_setting("sshd_wbfp", 0, &i_bfp_mode);
				if (ret != 0)
					return ret;
			}
		}
#endif
#if defined (APP_FTPD)
		if (is_ftp_enabled) {
			ret = firewall_read_enabled_setting("ftpd_wopen", &is_ftpd_wopen);
			if (ret != 0)
				return ret;
			if (is_ftpd_wopen) {
				ret = firewall_read_integer_setting("ftpd_wport", 0, &i_ftpd_wport);
				if (ret != 0)
					return ret;
			}
		}
#endif
		ret = firewall_read_enabled_setting("udpxy_wopen", &is_udpxy_wopen);
		if (ret != 0)
			return ret;
		if (is_udpxy_wopen) {
			ret = firewall_read_integer_setting("udpxy_enable_x", 0, &i_udpxy_port);
			if (ret != 0)
				return ret;
			if (firewall_unprivileged_port_is_safe(i_udpxy_port)) {
				ret = firewall_read_integer_setting("udpxy_wport", 0, &i_udpxy_wport);
				if (ret != 0)
					return ret;
			}
		}
#if defined (APP_TRMD)
		is_trmd_supported = is_torrent_support();
		if (is_trmd_supported) {
			ret = firewall_read_enabled_setting("trmd_enable", &is_trmd_enabled);
			if (ret != 0)
				return ret;
			if (is_trmd_enabled) {
				ret = firewall_read_integer_setting("trmd_pport", 0, &i_trmd_pport);
				if (ret != 0)
					return ret;
				if (i_trmd_pport < 1024 || i_trmd_pport > 65535)
					i_trmd_pport = TRANSMISSION_PPORT;
				ret = firewall_read_enabled_setting("trmd_ropen", &is_trmd_ropen);
				if (ret != 0)
					return ret;
				if (is_trmd_ropen) {
					ret = firewall_read_integer_setting("trmd_rport", 0, &i_trmd_rport);
					if (ret != 0)
						return ret;
					if (i_trmd_rport < 1024 || i_trmd_rport > 65535)
						i_trmd_rport = TRANSMISSION_RPORT;
				}
			}
		}
#endif
#if defined (APP_ARIA)
		is_aria_supported = is_aria_support();
		if (is_aria_supported) {
			ret = firewall_read_enabled_setting("aria_enable", &is_aria_enabled);
			if (ret != 0)
				return ret;
			if (is_aria_enabled) {
				ret = firewall_read_integer_setting("aria_pport", 0, &i_aria_pport);
				if (ret != 0)
					return ret;
				if (i_aria_pport < 1024 || i_aria_pport > 65535)
					i_aria_pport = ARIA_PPORT;
				ret = firewall_read_enabled_setting("aria_ropen", &is_aria_ropen);
				if (ret != 0)
					return ret;
				if (is_aria_ropen) {
					ret = firewall_read_integer_setting("aria_rport", 0, &i_aria_rport);
					if (ret != 0)
						return ret;
					if (i_aria_rport < 1024 || i_aria_rport > 65535)
						i_aria_rport = ARIA_RPORT;
				}
			}
		}
#endif
	}

	is_logdrop = (strcmp(logdrop, IPT_CHAIN_NAME_LOG_DROP) == 0) ? 1 : 0;

	if (!(fp=firewall_open_rule_file(ipt_file, "IPv6 filter")))
		return 0;

	fprintf(fp, "*%s\n", "filter");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", (is_fw_enabled) ? "DROP" :  "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP6_FORWARD);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_BFP_LIMIT);
	if (i_mac_filter > 0)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_MAC_LIST);
	if (is_url_enabled)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_URL_LIST);
	if (is_logaccept)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LOG_ACCEPT);
	if (is_logdrop)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LOG_DROP);

	// maclist chain
	if (include_mac_filter(fp, i_mac_filter, i_mac_filter_count,
				    logdrop, &i_mac_filter) != 0) {
		saved_errno = errno ? errno : EIO;
		(void)fclose(fp);
		firewall_abort_core_restore_after_staging_failure();
		firewall_rule_file_remove(ipt_file);
		errno = saved_errno;
		return -1;
	}

	// INPUT chain (accept_source_route=0 by default, no needed drop RH0 packet)
	dtype = "INPUT";

	/* Policy for all traffic from MAC-filtered LAN clients */
	if (i_mac_filter > 0 && is_mac_filter_drop)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_MAC_LIST);

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", "ACCEPT");

	if (is_fw_enabled) {
		/* Accept all traffic from LAN clients */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");
		
		/* Accept all traffic from localhost */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, "lo", "ACCEPT");
		
		/* Accept ICMPv6 (ICMPv6 may be INVALID state!) */
		if (is_ping_enabled)
			fprintf(fp, "-A %s -p %d -j %s\n", dtype, 58, logaccept);
		else
			fprintf(fp, "-A %s -p %d ! --icmpv6-type %s -j %s\n", dtype, 58, "echo-request", logaccept);
		
		/* Drop the wrong state, INVALID, packets */
		fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", logdrop);
		
		/* Accept all multicast (do not log) */
		if (nvram_match("mr_enable_x", "1"))
			fprintf(fp, "-A %s -d ff00::/8 -j %s\n", dtype, "ACCEPT");
		
		/* Accept DHCPv6 */
		if (ipv6_type == IPV6_NATIVE_DHCP6)
			fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 547, 546, logaccept);
		
		// Firewall between WAN and Local
		
		/* http/https/ssh/ftp/udpxy accepted from wan only for original ports (no NAT in IPv6) */
		if ((i_http_proto == 0 || i_http_proto == 2) && is_misc_http_enabled) {
			wport = i_misc_httpport;
			lport = i_http_lanport;
			if (wport == lport)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
		}
#if defined (SUPPORT_HTTPS)
		wport = i_https_wport;
		lport = i_https_lport;
		if ((i_http_proto == 1 || i_http_proto == 2) && is_https_wopen && (wport == lport))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
#endif
#if defined (APP_SSHD)
		wport = i_sshd_wport;
		lport = 22;
		if (is_sshd_enabled && is_sshd_wopen && (wport == lport)) {
			i_bfplimit_ref++;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, IPT_CHAIN_NAME_BFP_LIMIT);
		}
#endif
#if defined (APP_FTPD)
		wport = i_ftpd_wport;
		lport = 21;
		if (is_ftp_enabled && is_ftpd_wopen && (wport == lport))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
#endif
		wport = i_udpxy_wport;
		lport = i_udpxy_port;
		if (firewall_unprivileged_port_is_safe(lport) && is_udpxy_wopen && (wport == lport))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
#if defined (APP_TRMD)
		if (is_trmd_enabled && is_trmd_supported) {
			wport = i_trmd_pport;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (is_trmd_ropen) {
				wport = i_trmd_rport;
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			}
		}
#endif
#if defined (APP_ARIA)
		if (is_aria_enabled && is_aria_supported) {
			wport = i_aria_pport;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (is_aria_ropen) {
				wport = i_aria_rport;
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			}
		}
#endif
		if (is_vpns_enabled) {
#if defined (APP_OPENVPN)
			if (i_vpns_type == 2) {
				const char *ov_prot = "udp";

				
				if (i_vpns_ov_prot == 1 || i_vpns_ov_prot == 3 || i_vpns_ov_prot == 5)
					ov_prot = "tcp";
				if (i_vpns_ov_prot == 2 || i_vpns_ov_prot == 3 || i_vpns_ov_prot == 4 || i_vpns_ov_prot == 5)
					fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, ov_prot, i_vpns_ov_port, logaccept);
			} else
#endif
			if (i_vpns_type == 1) {
				fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, 1701, logaccept);
			} else {
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, 1723, logaccept);
				fprintf(fp, "-A %s -p %d -j %s\n", dtype, 47, logaccept);
			}
		}
		
		/* Accept UDP ports 33434-33534 for traceroute */
		if (is_ping_enabled)
			fprintf(fp, "-A %s -p udp --dport %d:%d -j %s\n", dtype, 33434, 33534, logaccept);
		
		/* Drop all (only for log) */
		if (is_logdrop)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	// FORWARD chain (accept_source_route=0 by default, no needed drop RH0 packet)
	dtype = "FORWARD";

	if (i_mac_filter > 0)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_MAC_LIST);

	if (is_fw_enabled) {
		/* Pass the redirect, might be seen as INVALID, packets */
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, lan_if, lan_if, "ACCEPT");
	}

	/* use url filter before accepting ESTABLISHED packets */
	if (is_url_enabled) {
		int url_status = include_webstr_filter(fp, i_url_count);
		if (url_status < 0) {
			saved_errno = errno ? errno : EIO;
			logmessage(LOGNAME,
				   "IPv6 filter: aborting incomplete URL-filter ruleset (errno=%d)",
				   saved_errno);
			(void)fclose(fp);
			firewall_abort_core_restore_after_staging_failure();
			firewall_rule_file_remove(ipt_file);
			errno = saved_errno;
			return -1;
		}
		if (url_status > 0) {
			char mac_buf[24] = {0};
			timematch[0] = 0;
			if (timematch_conv(timematch, "url_date_x", "url_time_x")) {
				mac_conv("url_mac_x", -1, mac_buf);
				if (strlen(mac_buf) == 17) {
					strcat(timematch, " -m mac");
					if (is_url_inverse)
						strcat(timematch, " !");
					strcat(timematch, " --mac-source ");
					strcat(timematch, mac_buf);
				}
				fprintf(fp, "-A %s -i %s%s -j %s\n", dtype, lan_if, timematch, IPT_CHAIN_NAME_URL_LIST);
				ret |= MODULE_WEBSTR_MASK;
			}
		}
	}

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
	if ((ipv6_type != IPV6_NATIVE_STATIC && ipv6_type != IPV6_NATIVE_DHCP6) || tcp_mss_need)
		fprintf(fp, "-A %s%s -o %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype, " !", lan_if);

	/* Pass related connections, skip rest of checks */
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", "ACCEPT");

	if (is_fw_enabled) {
		/* Pass ICMPv6 (ICMPv6 may be in INVALID state!) */
		if (is_ping_enabled)
			fprintf(fp, "-A %s -p %d -j %s\n", dtype, 58, logaccept);
		else
			fprintf(fp, "-A %s -p %d ! --icmpv6-type %s -j %s\n", dtype, 58, "echo-request", logaccept);
		
		/* Drop all packets in the INVALID state */
		fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", logdrop);
		
		/* Pass multicast (do not log) */
		if (nvram_match("mr_enable_x", "1"))
			fprintf(fp, "-A %s -d ff00::/8 -j %s\n", dtype, "ACCEPT");
#if 0
		/* Filter out invalid WAN->WAN connections */
		if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
			fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, IFNAME_SIT, lan_if, logdrop);
		fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, wan_if, lan_if, logdrop);
		if (!is_wan_ipv6_if_ppp() && strcmp(man_if, wan_if))
			fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, man_if, lan_if, logdrop);
#endif
		/* Accept LAN other outbound traffic  */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");
		
		/* Jump to IPv6 pinhole (IGDv2 IP6FC or PCP) */
		if (is_upnp_enabled)
			fprintf(fp, "-A %s -j %s\n", dtype, MINIUPNPD_CHAIN_IP6_FORWARD);
		
		/* Drop all (only for log) */
		if (is_logdrop)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	// bfplimit chain
	dtype = IPT_CHAIN_NAME_BFP_LIMIT;
	if (i_bfplimit_ref) {
		if (i_bfp_mode > 0) {
			fprintf(fp, "-A %s -m recent --set --name %s\n", dtype, "blacklist6");
			fprintf(fp, "-A %s -m recent --update --hitcount %d --seconds %d --name %s -j %s\n", dtype, 4, get_sshd_bfp_time(i_bfp_mode), "blacklist6", logdrop);
		}
		fprintf(fp, "-A %s -j %s\n", dtype, logaccept);
	}

	// logaccept chain
	if (is_logaccept) {
		dtype = IPT_CHAIN_NAME_LOG_ACCEPT;
		ftype = "ACCEPT";
		fprintf(fp, "-A %s -m %s %s -j LOG --log-prefix \"%s \" "
			    "--log-tcp-options --log-ip-options\n", dtype, CT_STATE, "NEW", ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, ftype);
	}

	// logdrop chain
	if (is_logdrop) {
		dtype = IPT_CHAIN_NAME_LOG_DROP;
		ftype = "DROP";
		fprintf(fp, "-A %s -m %s %s -j LOG --log-prefix \"%s \" "
			    "--log-tcp-options --log-ip-options\n", dtype, CT_STATE, "NEW,INVALID", ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, ftype);
	}

	fprintf(fp, "COMMIT\n\n");
	if (firewall_finish_rule_file(fp, ipt_file, "IPv6 filter") == 0) {
		if (ret & MODULE_WEBSTR_MASK)
			module_smart_load("xt_webstr", NULL);
		firewall_apply_core_restore("IPv6 filter restore", "ip6tables", "filter", ipt_file);
	}
	firewall_rule_file_remove(ipt_file);

	return ret;
}

int
ip6t_filter_default(void)
{
	FILE *fp;
	char *ftype, *dtype;
	int is_fw_enabled, result = -1, firewall_status;
	char ipt_file[] = "/tmp/ip6t_filter.XXXXXX";

	firewall_status = firewall_read_enabled_setting("fw_enable_x", &is_fw_enabled);
	if (firewall_status != 0)
		return firewall_status;

	if (!(fp=firewall_open_rule_file(ipt_file, "IPv6 default filter")))
		return -1;

	fprintf(fp, "*%s\n", "filter");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", (is_fw_enabled) ? "DROP" :  "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP6_FORWARD);

	// INPUT chain
	dtype = "INPUT";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, "lo", ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -p %d ! --icmpv6-type %s -j %s\n", dtype, 58, "echo-request", ftype);
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", "DROP");
	if (is_fw_enabled)
		fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 547, 546, ftype);

	// FORWARD chain
	dtype = "FORWARD";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", ftype);
	fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, IFNAME_BR, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -p %d -j %s\n", dtype, 58, ftype);
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", "DROP");
	if (is_fw_enabled)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);

	fprintf(fp, "COMMIT\n\n");
	if (firewall_finish_rule_file(fp, ipt_file, "IPv6 default filter") == 0)
		result = firewall_apply_core_restore("IPv6 default filter restore", "ip6tables", "filter", ipt_file);
	firewall_rule_file_remove(ipt_file);
	return result;
}

static void
ip6t_mangle_rules(char *man_if)
{
	FILE *fp;
	int should_restore;
	char ipt_file[] = "/tmp/ip6t_mangle.XXXXXX";

	if (firewall_module_table_restore_required("IPv6 mangle restore",
						   "ip6table_mangle", 0,
						   &should_restore) != 0 || !should_restore)
		return;
	if (!(fp=firewall_open_rule_file(ipt_file, "IPv6 mangle")))
		return;

	fprintf(fp, "*%s\n", "mangle");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");

	fprintf(fp, "COMMIT\n\n");
	if (firewall_finish_rule_file(fp, ipt_file, "IPv6 mangle") == 0 && should_restore)
		firewall_apply_core_restore("IPv6 mangle restore", "ip6tables", "mangle", ipt_file);
	firewall_rule_file_remove(ipt_file);
}

static void
ip6t_nat_rules(char *man_if)
{
	FILE *fp;
	char ipt_file[] = "/tmp/ip6t_nat.XXXXXX";

	if (!(fp=firewall_open_rule_file(ipt_file, "IPv6 NAT")))
		return;

	fprintf(fp, "*%s\n", "nat");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");
	fprintf(fp, "-A POSTROUTING -s fc00:101:101::1/64 -j FULLCONENAT\n");
	fprintf(fp, "COMMIT\n\n");
	if (firewall_finish_rule_file(fp, ipt_file, "IPv6 NAT") == 0)
		firewall_apply_core_restore("IPv6 NAT restore", "ip6tables", "nat", ipt_file);
	firewall_rule_file_remove(ipt_file);
}


#endif

static int
ipt_nat_rules(char *man_if, char *man_ip,
              char *wan_if, char *wan_ip,
              char *lan_if, char *lan_ip, char *lan_net,
              int use_man, int wan_proto)
{
	FILE *fp;
	char *dtype, *vpnc_if;
	char ipt_file[] = "/tmp/ipt_nat.XXXXXX";
	char dmz_ip[16] = {0};
	int is_use_dmz = 0;
	int wport, lport, is_nat_enabled, is_fw_enabled, is_upnp_enabled = 0;
	int i_vpns_enable, i_vpnc_enable, i_vpns_type, i_vpnc_type, i_vpnc_sfw, i_http_proto;
	int i_misc_httpport = 0, i_http_lanport = 0;
	int i_udpxy_port = 0, i_udpxy_wport = 0;
#if defined (SUPPORT_HTTPS)
	int i_https_wport = 0, i_https_lport = 0;
#endif
#if defined (APP_SSHD)
	int i_sshd_wport = 0;
#endif
#if defined (APP_FTPD)
	int i_ftpd_wport = 0;
#endif
#if defined (APP_TRMD)
	int is_trmd_supported = 0, is_trmd_enabled = 0, is_trmd_ropen = 0;
	int i_trmd_pport = 0, i_trmd_rport = 0;
#endif
#if defined (APP_ARIA)
	int is_aria_supported = 0, is_aria_enabled = 0, is_aria_ropen = 0;
	int i_aria_pport = 0, i_aria_rport = 0;
#endif
	int firewall_status, i_vpns_vuse = 0, i_vpns_actl = 0;
	int is_vts_enabled = 0, i_vts_count = 0;
#if defined (APP_SSHD)
	int is_sshd_enabled = 1;
#endif
#if defined (APP_FTPD)
	int is_ftp_enabled = 1;
#endif
#if defined (APP_OPENVPN)
	int i_vpns_ov_mode = 0, i_vpns_ov_prot = 0, i_vpns_ov_port = 1194;
	int i_vpnc_ov_mode = 0, i_vpnc_ov_cnat = 0;
	int i_vpnc_ov_prot = 0, i_vpnc_ov_port = 1194;
#endif

	firewall_status = firewall_read_enabled_setting("wan_nat_x", &is_nat_enabled);
	if (firewall_status != 0)
		return firewall_status;
	if (is_nat_enabled) {
		firewall_status = firewall_read_enabled_setting("vts_enable_x", &is_vts_enabled);
		if (firewall_status != 0)
			return firewall_status;
		if (is_vts_enabled) {
			firewall_status = firewall_read_vts_entry_count(&i_vts_count);
			if (firewall_status != 0)
				return firewall_status;
		}
	}
	firewall_status = firewall_read_enabled_setting("fw_enable_x", &is_fw_enabled);
	if (firewall_status != 0)
		return firewall_status;
	is_upnp_enabled = 0;
	if (is_nat_enabled) {
		firewall_status = firewall_read_not_zero_setting("upnp_enable_x", &is_upnp_enabled);
		if (firewall_status != 0)
			return firewall_status;
	}

	firewall_status = firewall_read_integer_setting("vpns_enable", 0, &i_vpns_enable);
	if (firewall_status != 0)
		return firewall_status;
	firewall_status = firewall_read_integer_setting("vpnc_enable", 0, &i_vpnc_enable);
	if (firewall_status != 0)
		return firewall_status;
	firewall_status = firewall_read_integer_setting("vpns_type", 0, &i_vpns_type);
	if (firewall_status != 0)
		return firewall_status;
	firewall_status = firewall_read_integer_setting("vpnc_type", 0, &i_vpnc_type);
	if (firewall_status != 0)
		return firewall_status;
	firewall_status = firewall_read_integer_setting("vpnc_sfw", 0, &i_vpnc_sfw);
	if (firewall_status != 0)
		return firewall_status;

#if defined (APP_OPENVPN)
	if (i_vpnc_enable && i_vpnc_type == 2) {
		firewall_status = firewall_read_integer_setting("vpnc_ov_mode", 0, &i_vpnc_ov_mode);
		if (firewall_status != 0)
			return firewall_status;
		firewall_status = firewall_read_integer_setting("vpnc_ov_cnat", 0, &i_vpnc_ov_cnat);
		if (firewall_status != 0)
			return firewall_status;
	}
	if (is_nat_enabled && i_vpns_enable && i_vpns_type == 2) {
		firewall_status = firewall_read_integer_setting("vpns_ov_mode", 0, &i_vpns_ov_mode);
		if (firewall_status != 0)
			return firewall_status;
		firewall_status = firewall_read_integer_setting("vpns_ov_prot", 0, &i_vpns_ov_prot);
		if (firewall_status != 0)
			return firewall_status;
		firewall_status = firewall_read_integer_setting("vpns_ov_port", 1194, &i_vpns_ov_port);
		if (firewall_status != 0)
			return firewall_status;
		if (i_vpns_ov_port < 1 || i_vpns_ov_port > 65535)
			i_vpns_ov_port = 1194;
	}
	if (is_nat_enabled && i_vpnc_enable && i_vpnc_type == 2) {
		firewall_status = firewall_read_integer_setting("vpnc_ov_prot", 0, &i_vpnc_ov_prot);
		if (firewall_status != 0)
			return firewall_status;
		firewall_status = firewall_read_integer_setting("vpnc_ov_port", 1194, &i_vpnc_ov_port);
		if (firewall_status != 0)
			return firewall_status;
		if (i_vpnc_ov_port < 1 || i_vpnc_ov_port > 65535)
			i_vpnc_ov_port = 1194;
	}
#endif

	if (is_nat_enabled && i_vpns_enable) {
		firewall_status = firewall_read_integer_setting("vpns_vuse", 0, &i_vpns_vuse);
		if (firewall_status != 0)
			return firewall_status;
		firewall_status = firewall_read_integer_setting("vpns_actl", 0, &i_vpns_actl);
		if (firewall_status != 0)
			return firewall_status;
	}

	if (is_nat_enabled && is_fw_enabled) {
#if defined (APP_SSHD)
		firewall_status = firewall_read_not_zero_setting("sshd_enable", &is_sshd_enabled);
		if (firewall_status != 0)
			return firewall_status;
#endif
#if defined (APP_FTPD)
		firewall_status = firewall_read_not_zero_setting("enable_ftp", &is_ftp_enabled);
		if (firewall_status != 0)
			return firewall_status;
#endif
#if defined (SUPPORT_HTTPS)
		firewall_status = firewall_read_integer_setting("http_proto", 0, &i_http_proto);
		if (firewall_status != 0)
			return firewall_status;
#else
		i_http_proto = 0;
#endif
		if ((i_http_proto == 0 || i_http_proto == 2) && nvram_match("misc_http_x", "1")) {
			firewall_status = firewall_read_integer_setting("misc_httpport_x", 0, &i_misc_httpport);
			if (firewall_status != 0)
				return firewall_status;
			firewall_status = firewall_read_integer_setting("http_lanport", 0, &i_http_lanport);
			if (firewall_status != 0)
				return firewall_status;
		}
#if defined (SUPPORT_HTTPS)
		if ((i_http_proto == 1 || i_http_proto == 2) && nvram_match("https_wopen", "1")) {
			firewall_status = firewall_read_integer_setting("https_wport", 0, &i_https_wport);
			if (firewall_status != 0)
				return firewall_status;
			firewall_status = firewall_read_integer_setting("https_lport", 0, &i_https_lport);
			if (firewall_status != 0)
				return firewall_status;
		}
#endif
#if defined (APP_SSHD)
		if (is_sshd_enabled && nvram_match("sshd_wopen", "1")) {
			firewall_status = firewall_read_integer_setting("sshd_wport", 0, &i_sshd_wport);
			if (firewall_status != 0)
				return firewall_status;
		}
#endif
#if defined (APP_FTPD)
		if (is_ftp_enabled && nvram_match("ftpd_wopen", "1")) {
			firewall_status = firewall_read_integer_setting("ftpd_wport", 0, &i_ftpd_wport);
			if (firewall_status != 0)
				return firewall_status;
		}
#endif
		if (nvram_match("udpxy_wopen", "1")) {
			firewall_status = firewall_read_integer_setting("udpxy_enable_x", 0, &i_udpxy_port);
			if (firewall_status != 0)
				return firewall_status;
			if (firewall_unprivileged_port_is_safe(i_udpxy_port)) {
				firewall_status = firewall_read_integer_setting("udpxy_wport", 0, &i_udpxy_wport);
				if (firewall_status != 0)
					return firewall_status;
			}
		}
	}


	vpnc_if = NULL;
	if (i_vpnc_enable) {
#if defined (APP_OPENVPN)
		if (i_vpnc_type == 2) {
			if (i_vpnc_ov_mode == 1)
				vpnc_if = IFNAME_CLIENT_TUN;
			else if (i_vpnc_ov_cnat == 1)
				vpnc_if = IFNAME_CLIENT_TAP;
		} else
#endif
			vpnc_if = IFNAME_CLIENT_PPP;
	}

	if (!firewall_ipv4_address_is_safe(wan_ip))
		wan_ip = NULL;

	if (!firewall_ipv4_address_is_safe(man_ip))
		man_ip = NULL;

	if (is_nat_enabled) {
		snprintf(dmz_ip, sizeof(dmz_ip), "%s", nvram_safe_get("dmz_ip"));
		is_use_dmz = firewall_ipv4_address_is_safe(dmz_ip) ? 1 : 0;
	}
	if (is_use_dmz) {
#if defined (APP_TRMD)
		is_trmd_supported = is_torrent_support();
		if (is_trmd_supported) {
			firewall_status = firewall_read_enabled_setting("trmd_enable", &is_trmd_enabled);
			if (firewall_status != 0)
				return firewall_status;
			if (is_trmd_enabled) {
				firewall_status = firewall_read_integer_setting("trmd_pport", 0, &i_trmd_pport);
				if (firewall_status != 0)
					return firewall_status;
				if (i_trmd_pport < 1024 || i_trmd_pport > 65535)
					i_trmd_pport = TRANSMISSION_PPORT;
				firewall_status = firewall_read_enabled_setting("trmd_ropen", &is_trmd_ropen);
				if (firewall_status != 0)
					return firewall_status;
				if (is_trmd_ropen) {
					firewall_status = firewall_read_integer_setting("trmd_rport", 0, &i_trmd_rport);
					if (firewall_status != 0)
						return firewall_status;
					if (i_trmd_rport < 1024 || i_trmd_rport > 65535)
						i_trmd_rport = TRANSMISSION_RPORT;
				}
			}
		}
#endif
#if defined (APP_ARIA)
		is_aria_supported = is_aria_support();
		if (is_aria_supported) {
			firewall_status = firewall_read_enabled_setting("aria_enable", &is_aria_enabled);
			if (firewall_status != 0)
				return firewall_status;
			if (is_aria_enabled) {
				firewall_status = firewall_read_integer_setting("aria_pport", 0, &i_aria_pport);
				if (firewall_status != 0)
					return firewall_status;
				if (i_aria_pport < 1024 || i_aria_pport > 65535)
					i_aria_pport = ARIA_PPORT;
				firewall_status = firewall_read_enabled_setting("aria_ropen", &is_aria_ropen);
				if (firewall_status != 0)
					return firewall_status;
				if (is_aria_ropen) {
					firewall_status = firewall_read_integer_setting("aria_rport", 0, &i_aria_rport);
					if (firewall_status != 0)
						return firewall_status;
					if (i_aria_rport < 1024 || i_aria_rport > 65535)
						i_aria_rport = ARIA_RPORT;
				}
			}
		}
#endif
	}

	if (!(fp=firewall_open_rule_file(ipt_file, "IPv4 NAT")))
		return 0;

	fprintf(fp, "*%s\n", "nat");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_VSERVER);
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_NAT);
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_NAT_POST);

	// VSERVER chain
	dtype = IPT_CHAIN_NAME_VSERVER;

	/* pre-routing from WAN */
	if (wan_ip)
		fprintf(fp, "-A PREROUTING -d %s -j %s\n", wan_ip, dtype);

	/* pre-routing from MAN */
	if (man_ip && use_man)
		fprintf(fp, "-A PREROUTING -d %s -j %s\n", man_ip, dtype);

	/* pre-routing from VPN client */
	if (vpnc_if && i_vpnc_sfw == 3)
		fprintf(fp, "-A PREROUTING -i %s ! -d %s -j %s\n", vpnc_if, lan_net, dtype);

	if (is_nat_enabled) {
		/* redirect all LAN clients' DNS queries (PREROUTING) */
		if (wan_ip && nvram_match("redirect_all_dns", "1")) {
			fprintf(fp, "-I %s -i %s -p tcp --dport 53 -j DNAT --to %s\n", "PREROUTING", lan_if, lan_ip);
			fprintf(fp, "-I %s -i %s -p udp --dport 53 -j DNAT --to %s\n", "PREROUTING", lan_if, lan_ip);
		}
		
		/* BattleNET (PREROUTING + POSTROUTING) */
		if (wan_ip && nvram_match("sp_battle_ips", "1")) {
			fprintf(fp, "-A %s -p udp -d %s --sport %d -j NETMAP --to %s\n", "PREROUTING", wan_ip, BATTLENET_PORT, lan_net);
			fprintf(fp, "-A %s -p udp -s %s --dport %d -j NETMAP --to %s\n", "POSTROUTING", lan_net, BATTLENET_PORT, wan_ip);
		}
		
#if 0
		/* miniupnpd postrouting chain (is really needed for XBox One?) */
		if (is_upnp_enabled)
			fprintf(fp, "-A %s -o %s -j %s\n", "POSTROUTING", wan_if, MINIUPNPD_CHAIN_IP4_NAT_POST);
#endif
		
		/* masquerade WAN connection for LAN clients */
		if (nvram_match("nf_nat_type", "1")) /* classical nat: 2, fullcone nat: 1*/
			include_masquerade(fp, wan_if, wan_ip, lan_net, 1);
		else
			include_masquerade(fp, wan_if, wan_ip, lan_net, 0);
		
		/* masquerade MAN connection for LAN clients */
		if (use_man) {
#if 0
			/* miniupnpd postrouting chain (is really needed for XBox One?) */
			if (is_upnp_enabled)
				fprintf(fp, "-A %s -o %s -j %s\n", "POSTROUTING", man_if, MINIUPNPD_CHAIN_IP4_NAT_POST);
#endif
			include_masquerade(fp, man_if, man_ip, lan_net, 0);
		}
		
		/* masquerade VPN client connection for LAN clients */
		if (vpnc_if && i_vpnc_sfw != 2)
			include_masquerade(fp, vpnc_if, NULL, lan_net, 0);
		
		/* masquerade WAN connection for VPN server clients */
		if (i_vpns_enable) {
			char vpn_net[24] = {0};
			ip2class(nvram_safe_get("vpns_vnet"), VPN_SERVER_SUBNET_MASK, vpn_net, sizeof(vpn_net));
			if (strcmp(vpn_net, lan_net) != 0) {
#if defined (APP_OPENVPN)
				if (i_vpns_type == 2) {
					if (i_vpns_ov_mode == 1) {
						if (i_vpns_actl == 0 || i_vpns_actl == 1 || i_vpns_actl == 4)
							include_masquerade(fp, wan_if, wan_ip, vpn_net, 0);
						
						/* masquerade VPN server clients to LAN */
						if (i_vpns_vuse == 2)
							include_masquerade(fp, lan_if, lan_ip, vpn_net, 0);
					}
				} else
#endif
				{
					if (i_vpns_vuse && (i_vpns_actl == 0 || i_vpns_actl == 1 || i_vpns_actl == 4))
						include_masquerade(fp, wan_if, wan_ip, vpn_net, 0);
					
					/* masquerade VPN server clients to LAN */
					if (i_vpns_vuse == 2)
						include_masquerade(fp, lan_if, lan_ip, vpn_net, 0);
				}
			}
		}
		
		/* masquerade LAN to LAN (NAT loopback) */
		if (nvram_match("nf_nat_loop", "1"))
			fprintf(fp, "-A POSTROUTING -o %s -s %s -d %s -j SNAT --to-source %s\n", lan_if, lan_net, lan_net, lan_ip);
		
		/* Local ports remap (http/https/ssh/ftp/udpxy) */
		if (is_fw_enabled) {

			if ((i_http_proto == 0 || i_http_proto == 2) && nvram_match("misc_http_x", "1")) {
				wport = i_misc_httpport;
				lport = i_http_lanport;
				if (wport < 80 || wport > 65535) {
					wport = 8080;
					nvram_set_int("misc_httpport_x", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
#if defined (SUPPORT_HTTPS)
			if ((i_http_proto == 1 || i_http_proto == 2) && nvram_match("https_wopen", "1")) {
				wport = i_https_wport;
				lport = i_https_lport;
				if (wport < 81 || wport > 65535) {
					wport = 8443;
					nvram_set_int("https_wport", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
#endif
#if defined (APP_SSHD)
			if (is_sshd_enabled && nvram_match("sshd_wopen", "1")) {
				wport = i_sshd_wport;
				lport = 22;
				if (wport < 22 || wport > 65535) {
					wport = 10022;
					nvram_set_int("sshd_wport", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
#endif
#if defined (APP_FTPD)
			if (is_ftp_enabled && nvram_match("ftpd_wopen", "1")) {
				wport = i_ftpd_wport;
				lport = 21;
				if (wport < 21 || wport > 65535) {
					wport = 21;
					nvram_set_int("ftpd_wport", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
#endif
			lport = i_udpxy_port;
			if (firewall_unprivileged_port_is_safe(lport) && nvram_match("udpxy_wopen", "1")) {
				wport = i_udpxy_wport;
				if (wport < 1024 || wport > 65535) {
					wport = lport;
					nvram_set_int("udpxy_wport", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
		}
		
		/* Port Forwarding (Virtual Server) mappings */
		if (is_vts_enabled)
			include_vts_nat(fp, i_vts_count);
		
		/* IGD UPnP mappings */
		if (is_upnp_enabled)
			fprintf(fp, "-A %s -j %s\n", dtype, MINIUPNPD_CHAIN_IP4_NAT);
		
		/* Exposed station (DMZ), skip local traffic first */
		if (is_use_dmz) {
			unsigned int vpn_proto_mask = 0;
#if defined (APP_OPENVPN)
			unsigned int ovpns_hash = 0, ovpnc_hash = 0;
#endif
			/* skip DMZ for local VPN server */
			if (i_vpns_enable) {
#if defined (APP_OPENVPN)
				if (i_vpns_type == 2) {
					int i_prot = i_vpns_ov_prot;
					
					/* OpenVPN IPv4 */
					if (i_prot < 2) {
						vpn_proto_mask |= 0x04;
						ovpns_hash = i_vpns_ov_port;
						if (i_prot == 1)
							ovpns_hash |= (1u << 16);
					}
				} else
#endif
				if (i_vpns_type == 1) {
					/* L2TP */
					vpn_proto_mask |= 0x02;
				} else {
					/* PPTP */
					vpn_proto_mask |= 0x01;
				}
			}
			
			/* skip DMZ for local VPN client */
			if (i_vpnc_enable) {
#if defined (APP_OPENVPN)
				if (i_vpnc_type == 2) {
					int i_prot = i_vpnc_ov_prot;
					
					/* OpenVPN IPv4 */
					if (i_prot < 2) {
						vpn_proto_mask |= 0x04;
						ovpnc_hash = i_vpnc_ov_port;
						if (i_prot == 1)
							ovpnc_hash |= (1u << 16);
					}
				} else
#endif
				if (i_vpnc_type == 1) {
					/* L2TP */
					vpn_proto_mask |= 0x02;
				} else {
					/* PPTP */
					vpn_proto_mask |= 0x01;
				}
			}
			
			/* skip DMZ for WAN VPN client */
			if (wan_proto == IPV4_WAN_PROTO_L2TP)
				vpn_proto_mask |= 0x02;
			else if (wan_proto == IPV4_WAN_PROTO_PPTP)
				vpn_proto_mask |= 0x01;
			
#if defined (APP_OPENVPN)
			if (vpn_proto_mask & 0x04) {
				if (ovpns_hash != 0) {
					const char *ov_prot = (ovpns_hash & (1u << 16)) ? "tcp" : "udp";
					fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n",
							dtype, ov_prot, (ovpns_hash & 0xffff));
				}
				if (ovpnc_hash != 0 && ovpnc_hash != ovpns_hash) {
					const char *ov_prot = (ovpnc_hash & (1u << 16)) ? "tcp" : "udp";
					fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n",
							dtype, ov_prot, (ovpnc_hash & 0xffff));
				}
			}
#endif
			if (vpn_proto_mask & 0x02) {
				fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "udp", 1701);
			}
			
			if (vpn_proto_mask & 0x01) {
				fprintf(fp, "-A %s -p %d -j RETURN\n", dtype, 47);
				fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "tcp", 1723);
			}
#if defined (USE_IPV6)
			/* skip DMZ for local IPv6 (SIT) */
			{
				int ipv6_type = get_ipv6_type();
				if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
					fprintf(fp, "-A %s -p %d -j RETURN\n", dtype, 41);
			}
#endif
#if defined (APP_TRMD)
			/* skip DMZ for local Transmission */
			if (is_trmd_enabled && is_trmd_supported) {
				wport = i_trmd_pport;
				if (wport < 1024 || wport > 65535) wport = TRANSMISSION_PPORT;
				fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "tcp", wport);
				fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "udp", wport);
				
				if (is_trmd_ropen) {
					wport = i_trmd_rport;
					if (wport < 1024 || wport > 65535) wport = TRANSMISSION_RPORT;
					fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "tcp", wport);
				}
			}
#endif
#if defined (APP_ARIA)
			/* skip DMZ for local Aria2 */
			if (is_aria_enabled && is_aria_supported) {
				wport = i_aria_pport;
				if (wport < 1024 || wport > 65535) wport = ARIA_PPORT;
				fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "tcp", wport);
				fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "udp", wport);
				
				if (is_aria_ropen) {
					wport = i_aria_rport;
					if (wport < 1024 || wport > 65535) wport = ARIA_RPORT;
					fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "tcp", wport);
				}
			}
#endif
			/* DNAT all other packets to DMZ host (except ICMP - use Port Forwarding to redirect ICMP) */
			fprintf(fp, "-A %s ! -p icmp -j DNAT --to %s\n", dtype, dmz_ip);
		}
	}

	fprintf(fp, "COMMIT\n\n");
	if (firewall_finish_rule_file(fp, ipt_file, "IPv4 NAT") == 0)
		firewall_apply_core_restore("IPv4 NAT restore", "iptables", "nat", ipt_file);
	firewall_rule_file_remove(ipt_file);

	return 0;
}

int
ipt_nat_default(void)
{
	FILE *fp;
	int is_nat_enabled, is_nat_loopback_enabled = 0;
	int result = -1, status;
	const char *lan_ip_setting = NULL, *lan_netmask_setting = NULL;
	char lan_ip[16] = {0}, lan_netmask[16] = {0};
	char ipt_file[] = "/tmp/ipt_nat.XXXXXX";

	status = firewall_read_enabled_setting("wan_nat_x", &is_nat_enabled);
	if (status != 0)
		return status;

	if (is_nat_enabled) {
		status = firewall_read_string_setting("lan_ipaddr", &lan_ip_setting);
		if (status != 0)
			return status;
		if (firewall_ipv4_address_is_safe(lan_ip_setting)) {
			snprintf(lan_ip, sizeof(lan_ip), "%s", lan_ip_setting);
			status = firewall_read_string_setting("lan_netmask", &lan_netmask_setting);
			if (status != 0)
				return status;
			if (!ipv4_netmask_is_contiguous(lan_netmask_setting)) {
				logmessage(LOGNAME, "Invalid LAN IPv4 netmask for default NAT; refusing restore");
				errno = EINVAL;
				return EINVAL;
			}
			snprintf(lan_netmask, sizeof(lan_netmask), "%s", lan_netmask_setting);
			status = firewall_read_enabled_setting("nf_nat_loop", &is_nat_loopback_enabled);
			if (status != 0)
				return status;
		}
	}

	if (!(fp=firewall_open_rule_file(ipt_file, "IPv4 default NAT")))
		return -1;

	fprintf(fp, "*%s\n", "nat");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_VSERVER);
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_NAT);
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_NAT_POST);

	if (is_nat_enabled && lan_ip[0]) {
		char *lan_if = IFNAME_BR;
		char lan_net[24] = {0};
		ip2class(lan_ip, lan_netmask, lan_net, sizeof(lan_net));

		/* masquerade lan to lan (NAT loopback) */
		if (is_nat_loopback_enabled) {
			fprintf(fp, "-A POSTROUTING -o %s -s %s -d %s -j SNAT --to-source %s\n",
				lan_if, lan_net, lan_net, lan_ip);
		}
	}

	fprintf(fp, "COMMIT\n\n");
	if (firewall_finish_rule_file(fp, ipt_file, "IPv4 default NAT") == 0)
		result = firewall_apply_core_restore("IPv4 default NAT restore", "iptables", "nat", ipt_file);
	firewall_rule_file_remove(ipt_file);
	return result;
}

static int
start_firewall_ex_locked(void)
{
	int unit, wan_proto, i_tcp_mss, i_use_man, i_rp;
	int status = 0;
	int hook_error, forward_status, rp_filter_write_status;
	char int_iptables_staged[256] = {0};
	char opt_iptables_staged[256] = {0};
#if defined (APP_SHADOWSOCKS)
	char shadowsocks_staged[256] = {0};
#endif
	char logaccept[16], logdrop[16];
	char wan_if[16], man_if[16], lan_if[16];
	const char *man_if_value, *wan_if_value;
	char wan_ip[16], man_ip[16], lan_ip[16], lan_net[24] = {0};
	const char *lan_netmask;
	const char *opt_iptables_script = "/opt/bin/update_iptables.sh";
	const char *int_iptables_script = SCRIPT_POST_FIREWALL;
#if defined (APP_SHADOWSOCKS)
	const char *shadowsocks_iptables_script = "/tmp/shadowsocks_iptables.save";
#endif
#if defined (APP_ADBYBY)
	const char *adbyby_iptables = "/home/root/.adbyby.save";
	int adbyby_snapshot_exists;
	int adbyby_restore_result;
#endif
	/* A previous cleanup failure or incomplete rollback is persistent
	 * recovery state. Do not discard its paths or start another ruleset apply. */
	if (firewall_restore_transaction_prepare_reset(&firewall_core_restore_transaction,
						firewall_remove_restore_snapshot, NULL) != 0) {
		if (firewall_core_restore_transaction.rollback_failed ||
		    firewall_core_restore_transaction.cleanup_failed)
			firewall_log_retained_restore_snapshots(&firewall_core_restore_transaction);
		logmessage(LOGNAME,
			   "Refusing firewall rebuild: previous recovery state remains unresolved (%lu table snapshots, %lu partial cleanup paths retained)",
			   (unsigned long)firewall_core_restore_transaction.snapshot_count,
			   (unsigned long)firewall_core_restore_transaction.cleanup_path_count);
		errno = EBUSY;
		return EBUSY;
	}
	firewall_core_restore_transaction_active = 0;
	unit = 0;

	snprintf(lan_if, sizeof(lan_if), "%s", IFNAME_BR);

	/* These names are interpolated into iptables-restore rule tokens. Validate
	 * NVRAM-backed values before copying, and validate the fallback output
	 * before use, so no newline or option-like token reaches a table writer. */
	man_if_value = get_man_ifname(unit);
	if (man_if_value && *man_if_value &&
	    !firewall_interface_name_is_safe(man_if_value)) {
		logmessage(LOGNAME, "Unsafe management interface token; refusing firewall rebuild");
		errno = EINVAL;
		return EINVAL;
	}
	snprintf(man_if, sizeof(man_if), "%s", man_if_value ? man_if_value : "");

	wan_if_value = get_wan_unit_value(unit, "ifname_t");
	if (wan_if_value && *wan_if_value) {
		if (!firewall_interface_name_is_safe(wan_if_value)) {
			logmessage(LOGNAME, "Unsafe WAN interface token; refusing firewall rebuild");
			errno = EINVAL;
			return EINVAL;
		}
		snprintf(wan_if, sizeof(wan_if), "%s", wan_if_value);
	} else {
		status = get_wan_ifname(wan_if);
		if (status != 0) {
			logmessage(LOGNAME, "No safe WAN interface token; refusing firewall rebuild (status=%d)",
				   status);
			errno = status;
			return status;
		}
	}

	if (!firewall_interface_name_is_safe(wan_if)) {
		logmessage(LOGNAME, "No safe WAN interface token; refusing firewall rebuild");
		errno = EINVAL;
		return EINVAL;
	}
	if (!firewall_interface_name_is_safe(lan_if)) {
		logmessage(LOGNAME, "No safe LAN interface token; refusing firewall rebuild");
		errno = EINVAL;
		return EINVAL;
	}

	snprintf(lan_ip, sizeof(lan_ip), "%s", nvram_safe_get("lan_ipaddr"));
	if (!firewall_ipv4_address_is_safe(lan_ip)) {
		logmessage(LOGNAME, "Invalid LAN IPv4 address for firewall; using 0.0.0.0");
		strcpy(lan_ip, "0.0.0.0");
	}
	snprintf(man_ip, sizeof(man_ip), "%s", nvram_safe_get("wanx_ipaddr"));
	snprintf(wan_ip, sizeof(wan_ip), "%s", get_wan_unit_value(unit, "ipaddr"));

	lan_netmask = nvram_safe_get("lan_netmask");
	if (!ipv4_netmask_is_contiguous(lan_netmask)) {
		logmessage(LOGNAME, "Invalid LAN IPv4 netmask for firewall; refusing rebuild");
		errno = EINVAL;
		return EINVAL;
	}
	ip2class(lan_ip, lan_netmask, lan_net, sizeof(lan_net));

	wan_proto = get_wan_proto(unit);
	i_tcp_mss = is_need_tcp_mss_wan(unit, wan_proto, man_if);

	i_use_man = 0;
	if (ppp_ifindex(wan_if) >= 0 && strcmp(man_if, wan_if)) {
		if (wan_proto != IPV4_WAN_PROTO_PPPOE || get_wan_unit_value_int(unit, "pppoe_man") > 0)
			i_use_man = 1;
	}

	if (i_use_man && !firewall_interface_name_is_safe(man_if)) {
		logmessage(LOGNAME, "No safe management interface token; refusing firewall rebuild");
		errno = EINVAL;
		return EINVAL;
	}

	/* mcast needs rp filter to be turned off only for non default iface */
	i_rp = 1;
	if (nvram_match("mr_enable_x", "1") || nvram_invmatch("udpxy_enable_x", "0")
#if defined (APP_XUPNPD)
	 || nvram_invmatch("xupnpd_enable_x", "0")
#endif
	    )
		i_rp = 0;
	/* Do not silently retain a stale rp_filter mode after a failed write.
	 * A missing interface is harmless for the restrictive default value (1),
	 * but multicast features that require value 0 must not proceed without it. */
	rp_filter_write_status = set_interface_conf_int("ipv4", man_if, "rp_filter", i_rp);
	status = firewall_rp_filter_write_status(
		man_if[0] && is_interface_exist(man_if), i_rp,
		rp_filter_write_status, errno);
	if (status != 0) {
		logmessage(LOGNAME,
			   "IPv4 rp_filter update failed for interface '%s' (value=%d, status=%d); aborting firewall rebuild",
			   man_if, i_rp, status);
		errno = status;
		return status;
	}

	/* Determine the log type */
	if (nvram_match("fw_log_x", "accept") || nvram_match("fw_log_x", "both"))
		strcpy(logaccept, IPT_CHAIN_NAME_LOG_ACCEPT);
	else
		strcpy(logaccept, "ACCEPT");

	if (nvram_match("fw_log_x", "drop") || nvram_match("fw_log_x", "both"))
		strcpy(logdrop, IPT_CHAIN_NAME_LOG_DROP);
	else
		strcpy(logdrop, "DROP");

	firewall_core_restore_transaction_active = 1;

	/* Snapshot each existing table immediately before its first core restore. */
	ipt_raw_rules();
	if (!firewall_restore_transaction_ok(&firewall_core_restore_transaction))
		goto firewall_restore_complete;
	ipt_mangle_rules(man_if, wan_if, i_use_man);
	if (!firewall_restore_transaction_ok(&firewall_core_restore_transaction))
		goto firewall_restore_complete;
	ipt_nat_rules(man_if, man_ip, wan_if, wan_ip, lan_if, lan_ip, lan_net, i_use_man, wan_proto);
	if (!firewall_restore_transaction_ok(&firewall_core_restore_transaction))
		goto firewall_restore_complete;
	ipt_filter_rules(man_if, wan_if, lan_if, lan_ip, logaccept, logdrop, i_tcp_mss);
	if (!firewall_restore_transaction_ok(&firewall_core_restore_transaction))
		goto firewall_restore_complete;

#if defined (USE_IPV6)
	ip6t_mangle_rules(man_if);
	if (!firewall_restore_transaction_ok(&firewall_core_restore_transaction))
		goto firewall_restore_complete;
	ip6t_nat_rules(man_if);
	if (!firewall_restore_transaction_ok(&firewall_core_restore_transaction))
		goto firewall_restore_complete;
	ip6t_filter_rules(man_if, wan_if, lan_if, logaccept, logdrop, i_tcp_mss);
	if (!firewall_restore_transaction_ok(&firewall_core_restore_transaction))
		goto firewall_restore_complete;
#endif

firewall_restore_complete:
	firewall_core_restore_transaction_active = 0;
	if (firewall_restore_transaction_ok(&firewall_core_restore_transaction)) {
		if (firewall_restore_transaction_finish(&firewall_core_restore_transaction,
						firewall_remove_restore_snapshot, NULL) != 0)
			status = EIO;
	} else {
		(void)firewall_restore_transaction_abort(&firewall_core_restore_transaction,
			firewall_restore_core_table, firewall_remove_restore_snapshot, NULL);
		status = EIO;
	}
	if (firewall_restore_transaction_status(&firewall_core_restore_transaction) != 0) {
		status = EIO;
		if (firewall_core_restore_transaction.cleanup_failed) {
			firewall_log_retained_restore_snapshots(&firewall_core_restore_transaction);
			logmessage(LOGNAME, "Firewall ruleset result incomplete: snapshot cleanup remains pending");
		}
	}

	if (firewall_restore_transaction_rollback_failed(&firewall_core_restore_transaction)) {
		firewall_log_retained_restore_snapshots(&firewall_core_restore_transaction);
		logmessage(LOGNAME,
			   "Firewall rollback incomplete; disabling forwarding and skipping remaining setup");
		forward_status = firewall_disable_forwarding("Core firewall rollback");
		if (forward_status != 0)
			logmessage(LOGNAME, "forwarding may remain enabled after incomplete firewall rollback (status=%d)",
				   forward_status);
		errno = EIO;
		return EIO;
	}
	if (firewall_restore_transaction_ok(&firewall_core_restore_transaction)) {
#if defined (APP_SHADOWSOCKS)
	/* Missing optional hooks are a no-op; configured failures reach the caller. */
	if (firewall_hook_file_stage(shadowsocks_iptables_script, "/home/root",
				     ".firewall-hook-", shadowsocks_staged,
				     sizeof(shadowsocks_staged)) == 0) {
		hook_error = firewall_log_command_failure("Shadowsocks firewall hook", doSystem("sh %s", shadowsocks_staged));
		status = rc_first_service_start_status(status, hook_error);
		if (firewall_hook_file_remove(shadowsocks_staged) != 0) {
			hook_error = firewall_log_hook_cleanup_failure("Shadowsocks firewall hook",
						       shadowsocks_staged, errno);
			status = rc_first_service_start_status(status, hook_error);
		}
	} else if (errno != ENOENT) {
		hook_error = firewall_log_hook_stage_failure("Shadowsocks firewall hook", errno);
		status = rc_first_service_start_status(status, hook_error);
	}
#endif
#if defined (APP_ADBYBY)
	adbyby_snapshot_exists = check_if_file_exist(adbyby_iptables);
	/* The saved multi-table file may commit filter before a later table fails.
	 * Snapshot both affected IPv4 tables and restore their complete prior state
	 * on any partial extension-restore failure. */
	adbyby_restore_result = firewall_adbyby_restore_policy_apply(
		adbyby_snapshot_exists, &firewall_adbyby_restore_transaction,
		firewall_adbyby_blockip_preflight,
		firewall_adbyby_restore_snapshot_command,
		firewall_adbyby_snapshot_table,
		firewall_adbyby_restore_table_command,
		firewall_remove_restore_snapshot,
		firewall_adbyby_report_restore_failure,
		(void *)adbyby_iptables);
	if (adbyby_restore_result != FIREWALL_ADBYBY_RESTORE_OK)
		status = EIO;
	if (adbyby_restore_result == FIREWALL_ADBYBY_RESTORE_RECOVERY_FAILED) {
		firewall_log_retained_adbyby_restore_snapshots(
			&firewall_adbyby_restore_transaction);
		logmessage(LOGNAME,
			   "Adbyby firewall rollback incomplete; disabling forwarding and aborting remaining extension hooks");
		forward_status = firewall_disable_forwarding("Adbyby firewall rollback");
		if (forward_status != 0)
			logmessage(LOGNAME, "forwarding may remain enabled after Adbyby rollback failure (status=%d)",
				   forward_status);
		errno = EIO;
		return EIO;
	}
#endif
	/* Storage-restored hooks are privileged programs: reject links and unsafe
	 * metadata, then execute only a root-private snapshot of the opened inode. */
	if (firewall_hook_file_stage(int_iptables_script, "/home/root", ".post-firewall-",
				     int_iptables_staged, sizeof(int_iptables_staged)) == 0) {
		hook_error = firewall_log_command_failure("Persistent post-firewall hook",
						  doSystem("%s", int_iptables_staged));
		status = rc_first_service_start_status(status, hook_error);
		if (firewall_hook_file_remove(int_iptables_staged) != 0) {
			hook_error = firewall_log_hook_cleanup_failure("Persistent post-firewall hook",
						       int_iptables_staged, errno);
			status = rc_first_service_start_status(status, hook_error);
		}
	} else if (errno != ENOENT) {
		hook_error = firewall_log_hook_stage_failure("Persistent post-firewall hook", errno);
		status = rc_first_service_start_status(status, hook_error);
	}

	/* /opt is externally managed storage; apply the same root-hook trust gate. */
	if (firewall_hook_file_stage(opt_iptables_script, "/home/root", ".opt-iptables-",
				     opt_iptables_staged, sizeof(opt_iptables_staged)) == 0) {
		hook_error = firewall_log_command_failure("Optware iptables update hook",
						  doSystem("%s update", opt_iptables_staged));
		status = rc_first_service_start_status(status, hook_error);
		if (firewall_hook_file_remove(opt_iptables_staged) != 0) {
			hook_error = firewall_log_hook_cleanup_failure("Optware iptables update hook",
						       opt_iptables_staged, errno);
			status = rc_first_service_start_status(status, hook_error);
		}
	} else if (errno != ENOENT) {
		hook_error = firewall_log_hook_stage_failure("Optware iptables update hook", errno);
		status = rc_first_service_start_status(status, hook_error);
	}
	} else {
		logmessage(LOGNAME, "Skipping extension firewall hooks after built-in table restore failure");
		/*
		 * Rollback restored the previous rules, not the requested new policy.
		 * Preserve the existing forwarding state and stop before enabling
		 * forwarding or running post-firewall setup against an unapplied ruleset.
		 */
		errno = status ? status : EIO;
		return status ? status : EIO;
	}

	/* enable IPv4 forward only after every built-in table was applied */
	forward_status = set_ipv4_forward(1);
	if (forward_status != 0)
		logmessage(LOGNAME, "failed to enable IPv4 forwarding after firewall restore (status=%d)",
			   forward_status);
	status = rc_first_service_start_status(status, forward_status);

	if (nvram_match("pppoemwan_enable", "1")) {
		hook_error = firewall_log_command_failure("PPPoE-MWAN detection",
						  doSystem("/usr/bin/detect.sh"));
		status = rc_first_service_start_status(status, hook_error);
	}
		
	/* try unload unused iptables modules */
	module_smart_unload("xt_webstr", 0);
	module_smart_unload("xt_HL", 0);
	module_smart_unload("iptable_raw", 0);
	module_smart_unload("iptable_mangle", 0);
	module_smart_unload("ip6table_mangle", 0);

	if (status != 0)
		errno = status;
	return status;
}

int
start_firewall_ex(void)
{
	int lock_fd, status, saved_errno;
	int release_status, release_errno;

	lock_fd = firewall_lifecycle_lock_acquire();
	if (lock_fd < 0) {
		saved_errno = errno ? errno : EIO;
		logmessage(LOGNAME,
			   "Firewall rebuild refused: cannot acquire lifecycle lock (errno=%d)",
			   saved_errno);
		errno = saved_errno;
		return saved_errno;
	}

	/* The restore transaction spans multiple tables; serialize the whole
	 * rebuild, not just each individual iptables-restore invocation. */
	errno = 0;
	status = start_firewall_ex_locked();
	saved_errno = errno;

	release_status = firewall_lifecycle_lock_release(lock_fd);
	if (release_status != 0) {
		release_errno = errno ? errno : EIO;
		logmessage(LOGNAME,
			   "Firewall rebuild completed but lifecycle lock release failed (errno=%d)",
			   release_errno);
		if (status == 0) {
			status = release_errno;
			saved_errno = release_errno;
		}
	}

	if (status != 0) {
		if (saved_errno <= 0)
			saved_errno = status > 0 ? status : EIO;
		errno = saved_errno;
	}
	return status;
}

#ifndef HTTPD_DAEMON_POLICY_H
#define HTTPD_DAEMON_POLICY_H

#include <errno.h>
#include "process_inventory.h"

#define HTTPD_START_INITIAL_WAIT_SECONDS 1
#define HTTPD_START_RETRY_WAIT_SECONDS 1
#define HTTPD_START_PROBE_ATTEMPTS 4

struct httpd_start_config {
	int http_proto;
	int http_port;
	int misc_http_x;
	int https_port;
	int https_wopen;
};

typedef int (*httpd_nvram_int_reader_fn)(void *context, const char *param_name,
					 int missing_value, int *value);

/*
 * Read every setting needed by the selected HTTP/HTTPS listeners before the
 * caller normalizes ports, publishes service state, or launches HTTPD.
 * Build into a local snapshot so any backend/parse error leaves *config
 * untouched and cannot be mistaken for an absent key.
 */
static inline int
httpd_start_config_read(httpd_nvram_int_reader_fn read_int, void *context,
		       int support_https, struct httpd_start_config *config)
{
	struct httpd_start_config next;
	int status;

	if (!read_int || !config ||
	    (support_https != 0 && support_https != 1))
		return EINVAL;

	next.http_proto = 0;
	next.http_port = 0;
	next.misc_http_x = 0;
	next.https_port = 0;
	next.https_wopen = 0;

	if (support_https) {
		status = read_int(context, "http_proto", 0, &next.http_proto);
		if (status != 0)
			return status;
	}

	if (next.http_proto == 0 || next.http_proto == 2) {
		status = read_int(context, "http_lanport", 0, &next.http_port);
		if (status != 0)
			return status;
		status = read_int(context, "misc_http_x", 0, &next.misc_http_x);
		if (status != 0)
			return status;
	}

	if (support_https &&
	    (next.http_proto == 1 || next.http_proto == 2)) {
		status = read_int(context, "https_lport", 0, &next.https_port);
		if (status != 0)
			return status;
		status = read_int(context, "https_wopen", 0, &next.https_wopen);
		if (status != 0)
			return status;
	}

	*config = next;
	return 0;
}

/* A successful _eval() only confirms that the launcher exited successfully;
 * the detached HTTPD child must also remain observable. */
static inline int
httpd_start_postcondition(int launcher_status,
			  enum process_presence process_presence)
{
	if (launcher_status != 0)
		return launcher_status;

	if (process_presence == PROCESS_PRESENCE_RUNNING)
		return 0;
	if (process_presence == PROCESS_PRESENCE_ABSENT)
		return ESRCH;
	return EIO;
}

/*
 * A post-launch UNKNOWN/invalid scan is not the same as confirmed absence:
 * the detached listener may already be live and must enter rollback.
 */
static inline int
httpd_start_postcondition_requires_recovery(enum process_presence process_presence)
{
	return process_presence != PROCESS_PRESENCE_RUNNING &&
	       process_presence != PROCESS_PRESENCE_ABSENT;
}

/* Only a complete scan that confirms absence authorizes configuration/start. */
static inline int
httpd_start_can_launch(enum process_presence process_presence)
{
	return process_presence == PROCESS_PRESENCE_ABSENT;
}

/* Preserve duplicate status separately from an unobservable process table. */
static inline int
httpd_start_precondition_status(enum process_presence process_presence)
{
	if (process_presence == PROCESS_PRESENCE_RUNNING)
		return EBUSY;
	if (process_presence == PROCESS_PRESENCE_ABSENT)
		return 0;
	return EIO;
}

/* Do not publish HTTPD as stopped while the old process may still be alive. */
static inline int
httpd_stop_policy_publish_stopped(int stop_status)
{
	return stop_status == 0;
}

/* After failed-start rollback, UNKNOWN is conservatively still running. */
static inline int
httpd_failed_start_preserve_running_state(enum process_presence after_stop)
{
	return after_stop != PROCESS_PRESENCE_ABSENT;
}

/* Never start a replacement until the old HTTPD process is confirmed gone. */
static inline int
httpd_restart_can_start(int stop_status)
{
	return stop_status == 0;
}

/* NVRAM writers report 0/-1; retain the backend errno on every failed write. */
static inline int
httpd_nvram_write_status(int api_result, int saved_errno)
{
	if (api_result == 0)
		return 0;
	return saved_errno > 0 ? saved_errno : EIO;
}

/*
 * Derive firewall work from the selected listeners before any listener port
 * normalization or daemon launch. HTTP is enabled for protocols 0/2 and
 * HTTPS for protocols 1/2; unrelated listener flags must not trigger refresh.
 */
static inline int
httpd_start_config_firewall_required(const struct httpd_start_config *config)
{
	int required = 0;

	if (!config)
		return 0;
	if (config->http_proto == 0 || config->http_proto == 2)
		required |= config->misc_http_x;
	if (config->http_proto == 1 || config->http_proto == 2)
		required |= config->https_wopen;
	return required;
}

/* Match the legacy short-circuit gate without collapsing NVRAM errors. */
static inline int
httpd_firewall_refresh_required(int restart_requested, int listener_requested,
				int firewall_enabled)
{
	return restart_requested != 0 && listener_requested != 0 &&
	       firewall_enabled != 0;
}

/* A failed firewall refresh cannot leave a possibly active public HTTP listener. */
static inline int
httpd_firewall_failure_requires_stop(int firewall_status,
				     enum process_presence after_state)
{
	return firewall_status != 0 &&
	       after_state != PROCESS_PRESENCE_ABSENT;
}

#endif /* HTTPD_DAEMON_POLICY_H */

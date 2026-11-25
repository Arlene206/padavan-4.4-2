/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DHCP6C_SCRIPT_RETRY_POLICY_H
#define DHCP6C_SCRIPT_RETRY_POLICY_H

#include <errno.h>
#include <stddef.h>

/* Publish a callback environment as one logical update. If any putenv()
 * fails, remove every earlier entry so the consumer never runs with a
 * partially populated option set. The caller still owns all strings.
 */
typedef int (*dhcp6c_script_env_put_fn)(char *);
typedef void (*dhcp6c_script_env_unset_fn)(const char *);

static inline int
dhcp6c_script_env_publish(char **envp,
			  dhcp6c_script_env_put_fn publish,
			  dhcp6c_script_env_unset_fn unset_value)
{
	size_t published;

	if (!envp || !publish || !unset_value) {
		errno = EINVAL;
		return -1;
	}
	for (published = 0; envp[published] != NULL; ++published) {
		if (publish(envp[published]) != 0) {
			int saved_errno = errno ? errno : ENOMEM;

			while (published > 0)
				unset_value(envp[--published]);
			errno = saved_errno;
			return -1;
		}
	}
	return 0;
}

/* Return a non-wrapping remaining interval for an absolute monotonic deadline. */
static inline unsigned long long
dhcp6_timer_remaining_ms(unsigned long long deadline, unsigned long long now)
{
	return deadline > now ? deadline - now : 0ULL;
}

/* Avoid tight callback-failure loops while retaining the DHCP event. */
#define DHCP6C_SCRIPT_RETRY_DELAY_MS 5000ULL

static inline int dhcp6c_script_retry_required(int script_status)
{
	return script_status != 0;
}

#endif /* DHCP6C_SCRIPT_RETRY_POLICY_H */

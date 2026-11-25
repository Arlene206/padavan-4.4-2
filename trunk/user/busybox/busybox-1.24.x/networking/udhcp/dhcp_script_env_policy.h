/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DHCP_SCRIPT_ENV_POLICY_H
#define DHCP_SCRIPT_ENV_POLICY_H

#include <errno.h>
#include <stddef.h>

typedef int (*dhcp_script_env_put_fn)(char *);
typedef void (*dhcp_script_env_unset_fn)(const char *);

static inline int
dhcp_script_env_publish(char **envp,
			dhcp_script_env_put_fn publish,
			dhcp_script_env_unset_fn unset_value)
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

#endif

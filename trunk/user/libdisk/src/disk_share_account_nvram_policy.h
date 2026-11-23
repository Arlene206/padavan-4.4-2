#ifndef PADAVAN_DISK_SHARE_ACCOUNT_NVRAM_POLICY_H
#define PADAVAN_DISK_SHARE_ACCOUNT_NVRAM_POLICY_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <router_account_nvram_policy.h>

/*
 * Account inventory reads are a single NVRAM transaction. get() returns
 * 1 for a present key, 0 for a missing key, and -1 for backend/close errors.
 * Values point into the shared cache and are copied before the next get().
 */
struct disk_share_account_nvram_ops {
	int (*begin)(void *context);
	int (*get)(void *context, const char *name, char **value);
	int (*end)(void *context);
	int (*valid_name)(const char *name);
	int (*reserved_name)(const char *name, const char *http_username);
};

static inline void
disk_share_account_inventory_free(int count, char **account_list)
{
	int i;

	if (!account_list)
		return;
	for (i = 0; i < count; ++i)
		free(account_list[i]);
	free(account_list);
}

/*
 * Read a coherent account inventory and fail closed on every NVRAM error.
 * Missing acc_num retains legacy empty-inventory behavior. A count above the
 * supported capacity is corruption, not a reason to hide extra accounts.
 */
static inline int
disk_share_account_inventory_read(
	const struct disk_share_account_nvram_ops *ops, void *context,
	int max_count, int *acc_num, char ***account_list)
{
	char http_username[128];
	char key[24], *value;
	char **items = NULL;
	int expected_count = 0, count = 0, seen_empty = 0;
	int started = 0, saved_errno = 0;
	int result, status, i, j;

	if (acc_num)
		*acc_num = 0;
	if (account_list)
		*account_list = NULL;
	if (!ops || !ops->begin || !ops->get || !ops->end ||
	    !ops->valid_name || !ops->reserved_name ||
	    !acc_num || !account_list || max_count < 0) {
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	if (ops->begin(context) != 0) {
		errno = errno ? errno : EIO;
		return -1;
	}
	started = 1;

	result = router_account_nvram_read_count(ops->get, context, "acc_num",
						 max_count, &expected_count);
	if (result != 0) {
		saved_errno = result;
		goto finish;
	}

	if (expected_count > 0) {
		result = router_account_nvram_read_copy(
			ops->get, context, "http_username", "",
			http_username, sizeof(http_username));
		if (result != 0) {
			saved_errno = result;
			goto finish;
		}

		items = (char **)calloc((size_t)expected_count, sizeof(char *));
		if (!items) {
			saved_errno = errno ? errno : ENOMEM;
			goto finish;
		}

		for (i = 0; i < expected_count; ++i) {
			int key_len;

			key_len = snprintf(key, sizeof(key), "acc_username%d", i);
			if (key_len < 0 || (size_t)key_len >= sizeof(key)) {
				saved_errno = ENAMETOOLONG;
				goto finish;
			}

			value = NULL;
			errno = 0;
			status = ops->get(context, key, &value);
			if (status < 0) {
				saved_errno = errno ? errno : EIO;
				goto finish;
			}
			if (status == 0) {
				seen_empty = 1;
				continue;
			}
			if (status != 1 || !value) {
				saved_errno = EIO;
				goto finish;
			}

			if (!*value) {
				seen_empty = 1;
				continue;
			}
			if (seen_empty || !ops->valid_name(value) ||
			    ops->reserved_name(value, http_username)) {
				saved_errno = EINVAL;
				goto finish;
			}

			for (j = 0; j < count; ++j) {
				if (strcmp(items[j], value) == 0) {
					saved_errno = EINVAL;
					goto finish;
				}
			}

			items[count] = (char *)malloc(strlen(value) + 1);
			if (!items[count]) {
				saved_errno = errno ? errno : ENOMEM;
				goto finish;
			}
			memcpy(items[count], value, strlen(value) + 1);
			++count;
		}
	}

finish:
	if (started) {
		errno = 0;
		if (ops->end(context) != 0 && saved_errno == 0)
			saved_errno = errno ? errno : EIO;
	}

	if (saved_errno != 0) {
		disk_share_account_inventory_free(count, items);
		errno = saved_errno;
		return -1;
	}

	if (count == 0) {
		free(items);
		items = NULL;
	}
	*acc_num = count;
	*account_list = items;
	return 0;
}

#define DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX 128

/*
 * Copy passwords that will move during account deletion before touching
 * NVRAM. A failed read/lease release must leave the caller with no partial
 * snapshot, so a transient backend error cannot replace a real password
 * with the empty string.
 */
static inline int
disk_share_account_password_shift_snapshot(
	const struct disk_share_account_nvram_ops *ops, void *context,
	int account_count, int target, int max_count,
	char passwords[][DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX])
{
	char key[24], *value;
	int started = 0, saved_errno = 0, key_length, status, i;

	if (!ops || !ops->begin || !ops->get || !ops->end ||
	    !passwords || account_count < 1 || account_count > max_count ||
	    target < 0 || target >= account_count || max_count < 1) {
		errno = EINVAL;
		return -1;
	}
	memset(passwords, 0,
	       (size_t)max_count * DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX);
	if (target >= account_count - 1)
		return 0;

	errno = 0;
	if (ops->begin(context) != 0) {
		errno = errno ? errno : EIO;
		return -1;
	}
	started = 1;

	for (i = target; i < account_count - 1; ++i) {
		key_length = snprintf(key, sizeof(key), "acc_password%d", i + 1);
		if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
			saved_errno = ENAMETOOLONG;
			break;
		}
		value = NULL;
		errno = 0;
		status = ops->get(context, key, &value);
		if (status < 0) {
			saved_errno = errno ? errno : EIO;
			break;
		}
		if (status == 0)
			continue; /* Preserve the legacy empty-value behavior. */
		if (status != 1 || !value) {
			saved_errno = EIO;
			break;
		}
		if (strlen(value) >= DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX) {
			saved_errno = ENAMETOOLONG;
			break;
		}
		memcpy(passwords[i], value, strlen(value) + 1);
	}

	if (started) {
		errno = 0;
		if (ops->end(context) != 0 && saved_errno == 0)
			saved_errno = errno ? errno : EIO;
	}
	if (saved_errno != 0) {
		memset(passwords, 0,
		       (size_t)max_count * DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX);
		errno = saved_errno;
		return -1;
	}
	return 0;
}

#endif /* PADAVAN_DISK_SHARE_ACCOUNT_NVRAM_POLICY_H */

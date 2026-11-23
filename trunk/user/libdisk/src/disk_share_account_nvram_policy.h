#ifndef PADAVAN_DISK_SHARE_ACCOUNT_NVRAM_POLICY_H
#define PADAVAN_DISK_SHARE_ACCOUNT_NVRAM_POLICY_H

#include <errno.h>
#include <limits.h>
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
	int (*set)(void *context, const char *name, const char *value);
	int (*commit)(void *context);
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

/*
 * Preserve nvram_commit_safe()'s legacy manual-commit rule while separating
 * an absent key from an unreadable/malformed backend result.
 */
static inline int
disk_share_account_commit_required(int read_status, const char *manual_value,
                                   int *commit_required)
{
	if (!commit_required) {
		errno = EINVAL;
		return -1;
	}
	*commit_required = 0;
	if (read_status == 0) {
		*commit_required = 1;
		return 0;
	}
	if (read_status < 0) {
		if (errno == 0)
			errno = EIO;
		return -1;
	}
	if (read_status != 1 || !manual_value) {
		errno = EIO;
		return -1;
	}
	*commit_required = (atoi(manual_value) != 1);
	return 0;
}

/*
 * Last-account removal resets incompatible modes to the ordinary shared
 * mode. Missing keys retain nvram_get_int()'s legacy zero default, while
 * failed or malformed reads must not silently rewrite configuration.
 */
static inline int
disk_share_account_mode_needs_default(int read_status, const char *value,
                                      int *needs_default)
{
	char *end;
	long parsed;

	if (!needs_default) {
		errno = EINVAL;
		return -1;
	}
	*needs_default = 0;
	if (read_status == 0) {
		*needs_default = 1;
		return 0;
	}
	if (read_status < 0) {
		if (errno == 0)
			errno = EIO;
		return -1;
	}
	if (read_status != 1 || !value) {
		errno = EIO;
		return -1;
	}
	errno = 0;
	parsed = strtol(value, &end, 10);
	if (errno == ERANGE || end == value || *end != '\0' ||
	    parsed < INT_MIN || parsed > INT_MAX) {
		errno = EINVAL;
		return -1;
	}
	*needs_default = ((int)parsed != 1 && (int)parsed != 3);
	return 0;
}

/*
 * Remove an account from one exclusive NVRAM transaction. Revalidate the
 * caller's inventory under the writer lease, snapshot every overwritten
 * value, and compensate all writes if a setter or flash commit fails.
 * A NULL value passed to set() removes a previously absent key on rollback.
 */
static inline int
disk_share_account_inventory_remove(
	const struct disk_share_account_nvram_ops *ops, void *context,
	int expected_count, int target, int max_count, char *const *expected_users)
{
	char (*old_passwords)[DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX] = NULL;
	unsigned char *password_present = NULL;
	char key[24], count_value[16], old_count_value[16];
	char current_user[128], old_mode[2][32];
	const char *mode_names[2] = { "st_samba_mode", "st_ftp_mode" };
	char *value = NULL;
	int mode_present[2] = { 0, 0 };
	int mode_default[2] = { 0, 0 };
	int mode_attempted[2] = { 0, 0 };
	int started = 0, writes_started = 0;
	int status, result, saved_errno = 0, rollback_errno = 0;
	int current_count = 0, commit_required = 0;
	int key_length, i, j, mode_value_length;

	if (!ops || !ops->begin || !ops->get || !ops->set || !ops->commit ||
	    !ops->end || !expected_users || expected_count < 1 ||
	    expected_count > max_count || target < 0 || target >= expected_count ||
	    max_count < 1) {
		errno = EINVAL;
		return -1;
	}

	old_passwords = (char (*)[DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX])
		calloc((size_t)max_count, sizeof(*old_passwords));
	password_present = (unsigned char *)calloc((size_t)max_count, 1);
	if (!old_passwords || !password_present) {
		saved_errno = errno ? errno : ENOMEM;
		goto finish;
	}

	errno = 0;
	if (ops->begin(context) != 0) {
		saved_errno = errno ? errno : EIO;
		goto finish;
	}
	started = 1;

	result = router_account_nvram_read_count(ops->get, context, "acc_num",
						 max_count, &current_count);
	if (result != 0) {
		saved_errno = result;
		goto transaction_finish;
	}
	if (current_count != expected_count) {
		saved_errno = EAGAIN;
		goto transaction_finish;
	}

	/* Reject a stale account list instead of compacting another writer's data. */
	for (i = 0; i < expected_count; ++i) {
		key_length = snprintf(key, sizeof(key), "acc_username%d", i);
		if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
			saved_errno = ENAMETOOLONG;
			goto transaction_finish;
		}
		result = router_account_nvram_read_copy(
			ops->get, context, key, "", current_user, sizeof(current_user));
		if (result != 0) {
			saved_errno = result;
			goto transaction_finish;
		}
		if (!expected_users[i] || strcmp(current_user, expected_users[i]) != 0) {
			saved_errno = EAGAIN;
			goto transaction_finish;
		}
	}

	/* Snapshot every password slot that this delete will overwrite. */
	for (i = target; i < expected_count; ++i) {
		key_length = snprintf(key, sizeof(key), "acc_password%d", i);
		if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
			saved_errno = ENAMETOOLONG;
			goto transaction_finish;
		}
		value = NULL;
		errno = 0;
		status = ops->get(context, key, &value);
		if (status < 0) {
			saved_errno = errno ? errno : EIO;
			goto transaction_finish;
		}
		if (status == 0)
			continue;
		if (status != 1 || !value) {
			saved_errno = EIO;
			goto transaction_finish;
		}
		mode_value_length = (int)strlen(value);
		if (mode_value_length >= DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX) {
			saved_errno = ENAMETOOLONG;
			goto transaction_finish;
		}
		memcpy(old_passwords[i], value, (size_t)mode_value_length + 1);
		password_present[i] = 1;
	}

	/* Last-account mode changes also need exact rollback snapshots. */
	if (expected_count == 1) {
		for (j = 0; j < 2; ++j) {
			value = NULL;
			errno = 0;
			status = ops->get(context, mode_names[j], &value);
			if (disk_share_account_mode_needs_default(
				    status, value, &mode_default[j]) != 0) {
				saved_errno = errno ? errno : EIO;
				goto transaction_finish;
			}
			if (status == 1) {
				mode_value_length = (int)strlen(value);
				if (mode_value_length >= (int)sizeof(old_mode[j])) {
					saved_errno = ENAMETOOLONG;
					goto transaction_finish;
				}
				memcpy(old_mode[j], value, (size_t)mode_value_length + 1);
				mode_present[j] = 1;
			}
		}
	}

	value = NULL;
	errno = 0;
	status = ops->get(context, "nvram_manual", &value);
	if (disk_share_account_commit_required(status, value, &commit_required) != 0) {
		saved_errno = errno ? errno : EIO;
		goto transaction_finish;
	}

	snprintf(old_count_value, sizeof(old_count_value), "%d", expected_count);
	snprintf(count_value, sizeof(count_value), "%d", expected_count - 1);

	/* Mark the first setter attempted before calling into the ioctl adapter. */
	writes_started = 1;
	errno = 0;
	if (ops->set(context, "acc_num", count_value) != 0) {
		saved_errno = errno ? errno : EIO;
		goto rollback;
	}

	for (i = target; i < expected_count - 1; ++i) {
		key_length = snprintf(key, sizeof(key), "acc_username%d", i);
		if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
			saved_errno = ENAMETOOLONG;
			goto rollback;
		}
		errno = 0;
		if (ops->set(context, key, expected_users[i + 1]) != 0) {
			saved_errno = errno ? errno : EIO;
			goto rollback;
		}

		key_length = snprintf(key, sizeof(key), "acc_password%d", i);
		if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
			saved_errno = ENAMETOOLONG;
			goto rollback;
		}
		/* Preserve the legacy safe_get() behavior for a missing source slot. */
		errno = 0;
		if (ops->set(context, key, password_present[i + 1] ?
			     old_passwords[i + 1] : "") != 0) {
			saved_errno = errno ? errno : EIO;
			goto rollback;
		}
	}

	if (expected_count == 1) {
		for (j = 0; j < 2; ++j) {
			if (!mode_default[j])
				continue;
			mode_attempted[j] = 1;
			errno = 0;
			if (ops->set(context, mode_names[j], "1") != 0) {
				saved_errno = errno ? errno : EIO;
				goto rollback;
			}
		}
	}

	if (commit_required) {
		errno = 0;
		if (ops->commit(context) != 0) {
			saved_errno = errno ? errno : EIO;
			goto rollback;
		}
	}
	goto transaction_finish;

rollback:
	/*
	 * A failed ioctl/close can have changed the key despite returning error.
	 * Restore the complete pre-delete snapshot, not just successful setters.
	 */
	if (writes_started) {
		result = ops->set(context, "acc_num", old_count_value);
		if (result != 0 && rollback_errno == 0)
			rollback_errno = errno ? errno : EIO;

		for (i = target; i < expected_count - 1; ++i) {
			key_length = snprintf(key, sizeof(key), "acc_username%d", i);
			if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
				if (rollback_errno == 0)
					rollback_errno = ENAMETOOLONG;
			} else {
				result = ops->set(context, key, expected_users[i]);
				if (result != 0 && rollback_errno == 0)
					rollback_errno = errno ? errno : EIO;
			}

			key_length = snprintf(key, sizeof(key), "acc_password%d", i);
			if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
				if (rollback_errno == 0)
					rollback_errno = ENAMETOOLONG;
			} else {
				result = ops->set(context, key,
						  password_present[i] ? old_passwords[i] : NULL);
				if (result != 0 && rollback_errno == 0)
					rollback_errno = errno ? errno : EIO;
			}
		}

		for (j = 0; j < 2; ++j) {
			if (!mode_attempted[j])
				continue;
			result = ops->set(context, mode_names[j],
					  mode_present[j] ? old_mode[j] : NULL);
			if (result != 0 && rollback_errno == 0)
				rollback_errno = errno ? errno : EIO;
		}
		if (commit_required && ops->commit(context) != 0 && rollback_errno == 0)
			rollback_errno = errno ? errno : EIO;
	}
	if (rollback_errno != 0)
		saved_errno = EIO;

transaction_finish:
	if (started) {
		errno = 0;
		if (ops->end(context) != 0 && saved_errno == 0)
			saved_errno = errno ? errno : EIO;
	}
finish:
	free(old_passwords);
	free(password_present);
	if (saved_errno != 0) {
		errno = saved_errno;
		return -1;
	}
	return 0;
}

#endif /* PADAVAN_DISK_SHARE_ACCOUNT_NVRAM_POLICY_H */

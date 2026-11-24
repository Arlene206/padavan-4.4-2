#ifndef VSFTPD_ACCOUNT_AUTH_POLICY_H
#define VSFTPD_ACCOUNT_AUTH_POLICY_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <router_account_nvram_policy.h>

struct vsftpd_account_auth_nvram_ops {
	int (*begin)(void *context);
	int (*get)(void *context, const char *name, char **value);
	int (*end)(void *context);
};

/*
 * Resolve one FTP credential pair under the same exclusive NVRAM transaction.
 * Individual nvram_get_* calls lock only one key at a time; account writers
 * publish count, username and password in a multi-key transaction. Holding
 * one lease across the complete lookup prevents authentication from mixing
 * generations while an account is added, removed, renamed or updated.
 *
 * Returns 1 for a matching account, 0 when absent, and -1 on malformed or
 * failed NVRAM access. The output password is empty on every non-success
 * path, including failure to release the transaction lease.
 */
static int
vsftpd_account_auth_password_read(
	const struct vsftpd_account_auth_nvram_ops *ops, void *context,
	const char *account, int max_count, char *password,
	size_t password_size)
{
	char key[24], username[ROUTER_ACCOUNT_NV_NAME_SIZE];
	char *value = NULL;
	int started = 0, saved_errno = 0, count = 0, found = -1;
	int result = 0, status, key_length, i;
	size_t length;

	if (!ops || !ops->begin || !ops->get || !ops->end ||
	    !account || !*account || max_count < 0 ||
	    !password || password_size == 0) {
		errno = EINVAL;
		return -1;
	}
	memset(password, 0, password_size);

	errno = 0;
	if (ops->begin(context) != 0) {
		errno = errno ? errno : EIO;
		return -1;
	}
	started = 1;

	result = router_account_nvram_read_count(
		ops->get, context, "acc_num", max_count, &count);
	if (result != 0) {
		saved_errno = result;
		goto finish;
	}

	for (i = 0; i < count; ++i) {
		key_length = snprintf(key, sizeof(key), "acc_username%d", i);
		if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
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
		if (status != 1 || !value) {
			saved_errno = status == 0 ? EINVAL : EIO;
			goto finish;
		}
		length = strlen(value);
		if (length == 0 || length >= sizeof(username)) {
			saved_errno = length == 0 ? EINVAL : ENAMETOOLONG;
			goto finish;
		}
		memcpy(username, value, length + 1);
		if (strcmp(username, account) == 0) {
			if (found >= 0) {
				saved_errno = EEXIST;
				goto finish;
			}
			found = i;
		}
	}

	if (found >= 0) {
		key_length = snprintf(key, sizeof(key), "acc_password%d", found);
		if (key_length < 0 || (size_t)key_length >= sizeof(key)) {
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
		if (status != 1 || !value) {
			saved_errno = status == 0 ? EACCES : EIO;
			goto finish;
		}
		length = strlen(value);
		if (length == 0 || length >= password_size) {
			saved_errno = length == 0 ? EACCES : ENAMETOOLONG;
			goto finish;
		}
		memcpy(password, value, length + 1);
		result = 1;
	}

finish:
	if (started) {
		errno = 0;
		if (ops->end(context) != 0 && saved_errno == 0)
			saved_errno = errno ? errno : EIO;
	}
	if (saved_errno != 0) {
		memset(password, 0, password_size);
		errno = saved_errno;
		return -1;
	}
	errno = 0;
	return result;
}

#endif /* VSFTPD_ACCOUNT_AUTH_POLICY_H */

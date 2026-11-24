#ifndef HTTPD_AUTH_NVRAM_POLICY_H
#define HTTPD_AUTH_NVRAM_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

typedef int (*httpd_auth_nvram_begin_fn)(void *context);
typedef int (*httpd_auth_nvram_get_fn)(void *context,
                                      const char *name, char **value);
typedef int (*httpd_auth_nvram_end_fn)(void *context);

/* A missing credential key retains the legacy empty-string default. */
static inline int
httpd_auth_nvram_read_status(int result, int saved_errno)
{
	if (result == 0 || result == 1)
		return 0;
	if (result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	return EIO;
}

static inline int
httpd_auth_nvram_copy_one(httpd_auth_nvram_get_fn get_value,
                          void *context, const char *name,
                          char *destination, size_t destination_size)
{
	char *value = NULL;
	size_t length;
	int result, saved_errno, status;

	errno = 0;
	result = get_value(context, name, &value);
	saved_errno = errno;
	status = httpd_auth_nvram_read_status(result, saved_errno);
	if (status != 0)
		return status;
	if (result == 0)
		value = "";
	else if (value == NULL)
		return EIO;

	length = strlen(value);
	if (length >= destination_size)
		return ENAMETOOLONG;
	memcpy(destination, value, length + 1);
	return 0;
}

/*
 * Read the username/password pair under one NVRAM transaction. Each value is
 * copied before the next ioctl can overwrite libnvram's shared cache. Any
 * read, validation or transaction-close failure invalidates the entire pair.
 */
static inline int
httpd_auth_nvram_snapshot_credentials(
        const char *username_name, const char *password_name,
        char *username, size_t username_size,
        char *password, size_t password_size,
        httpd_auth_nvram_begin_fn begin_transaction,
        httpd_auth_nvram_get_fn get_value,
        httpd_auth_nvram_end_fn end_transaction, void *context)
{
	int status, api_result, saved_errno, end_status;

	if (!username_name || !username_name[0] ||
	    !password_name || !password_name[0] ||
	    !username || username_size == 0 ||
	    !password || password_size == 0 ||
	    !begin_transaction || !get_value || !end_transaction) {
		if (username && username_size > 0)
			username[0] = '\0';
		if (password && password_size > 0)
			password[0] = '\0';
		return EINVAL;
	}

	username[0] = '\0';
	password[0] = '\0';

	errno = 0;
	api_result = begin_transaction(context);
	saved_errno = errno;
	if (api_result != 0)
		return saved_errno > 0 ? saved_errno : EIO;

	status = httpd_auth_nvram_copy_one(get_value, context, username_name,
					   username, username_size);
	if (status == 0)
		status = httpd_auth_nvram_copy_one(get_value, context, password_name,
					   password, password_size);

	errno = 0;
	end_status = end_transaction(context);
	saved_errno = errno;
	if (end_status != 0 && status == 0)
		status = saved_errno > 0 ? saved_errno : EIO;

	if (status != 0) {
		username[0] = '\0';
		password[0] = '\0';
	}
	return status;
}

/* Map passwd/shadow update failures to a stable errno-style result. */
static inline int
httpd_auth_account_update_status(int operation_status, int saved_errno)
{
	if (operation_status == 0)
		return 0;
	if (saved_errno > 0)
		return saved_errno;
	return operation_status > 0 ? operation_status : EIO;
}

#endif /* HTTPD_AUTH_NVRAM_POLICY_H */

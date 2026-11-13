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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <bcmnvram.h>

#include "nvram_linux.h"
#include "nvram_transaction_lock.h"

#define PATH_DEV_NVRAM	"/dev/nvram"

#define MAX_CACHE_LEN	(2*NVRAM_MAX_VALUE_LEN)
#define MIN_CACHE_RES	32

static int
nvram_get_checked_unlocked(const char *name, char **value)
{
	static int nvr_index = 0;
	static char nvr_cache[MAX_CACHE_LEN] = {0};
	int ret, i_cache, nvram_fd;
	int saved_errno = 0;
	anvram_ioctl_t nvr;

	if (!name || !value) {
		errno = EINVAL;
		return -1;
	}

	*value = NULL;
	nvram_fd = open(PATH_DEV_NVRAM, O_RDWR);
	if (nvram_fd < 0) {
		perror(PATH_DEV_NVRAM);
		return -1;
	}

	i_cache = nvr_index;
	if ((MAX_CACHE_LEN - i_cache) < MIN_CACHE_RES)
		i_cache = 0;

	nvr.size = sizeof(nvr);
	nvr.len_param = strlen(name);
	nvr.len_value = MAX_CACHE_LEN - i_cache;
	nvr.param = (char*)name;
	nvr.value = &nvr_cache[i_cache];
	nvr.is_temp = 0;

	ret = ioctl(nvram_fd, NVRAM_IOCTL_GET, &nvr);
	if (ret < 0 && errno == EOVERFLOW &&
	    nvr.len_value > 0 && nvr.len_value <= MAX_CACHE_LEN) {
		if ((MAX_CACHE_LEN - i_cache) < nvr.len_value)
			i_cache = 0;
		nvr.len_value = MAX_CACHE_LEN - i_cache;
		nvr.value = &nvr_cache[i_cache];
		ret = ioctl(nvram_fd, NVRAM_IOCTL_GET, &nvr);
	}

	if (ret < 0) {
		saved_errno = errno;
		perror(PATH_DEV_NVRAM);
		if (close(nvram_fd) < 0 && saved_errno == 0)
			saved_errno = errno;
		errno = saved_errno;
		return -1;
	}

	if (nvr.len_value < 1) {
		*value = NULL;
	} else {
		if (nvr.len_value > MAX_CACHE_LEN - i_cache) {
			close(nvram_fd);
			errno = EOVERFLOW;
			return -1;
		}
		*value = nvr.value;
		nvr_index = i_cache + nvr.len_value;
	}

	if (close(nvram_fd) < 0) {
		perror(PATH_DEV_NVRAM);
		*value = NULL;
		return -1;
	}

	return (*value != NULL) ? 1 : 0;
}


static int
nvram_api_lock_acquire(int exclusive)
{
	return nvram_transaction_lock_acquire(
		exclusive, NVRAM_TRANSACTION_LOCK_WAIT_SECONDS);
}

static int
nvram_api_lock_finish(int lock_fd, int operation_result)
{
	int saved_errno = errno;

	if (nvram_transaction_lock_release(lock_fd) != 0 &&
	    operation_result >= 0)
		return -1;
	errno = saved_errno;
	return operation_result;
}

int
nvram_get_checked(const char *name, char **value)
{
	int lock_fd, ret;

	if (!name || !value) {
		errno = EINVAL;
		return -1;
	}
	*value = NULL;
	lock_fd = nvram_api_lock_acquire(1);
	if (lock_fd < 0)
		return -1;
	ret = nvram_get_checked_unlocked(name, value);
	return nvram_api_lock_finish(lock_fd, ret);
}

static char *
nvram_get_(const char *name)
{
	char *value = NULL;

	if (nvram_get_checked(name, &value) != 1)
		return NULL;

	return value;
}

char *
nvram_get(const char *name)
{
	return nvram_get_(name);
}

char *
nvram_safe_get(const char *name)
{
	static const char nvram_empty[4] = {0};

	char *value = nvram_get_(name);
	if (value)
		return value;
	else
		return (char *)nvram_empty;
}

int
nvram_get_int(const char *name)
{
	char *value = nvram_get_(name);
	if (value)
		return atoi(value);
	else
		return 0;
}

int
nvram_safe_get_int(const char* name, int val_def, int val_min, int val_max)
{
	int i_value = nvram_get_int(name);
	if (i_value < val_min)
		i_value = val_def;
	else if (i_value > val_max)
		i_value = val_def;
	return i_value;
}

static int
nvram_getall_unlocked(char *buf, int count, int include_temp)
{
	int ret, saved_errno = 0, nvram_fd;
	anvram_ioctl_t nvr;

	if (!buf || count < 1 || include_temp < 0 || include_temp > 2) {
		errno = EINVAL;
		return -1;
	}

	nvram_fd = open(PATH_DEV_NVRAM, O_RDWR);
	if (nvram_fd < 0) {
		perror(PATH_DEV_NVRAM);
		return -1;
	}

	/* Get all variables */
	*buf = '\0';

	nvr.size = sizeof(nvr);
	nvr.len_param = 0;
	nvr.len_value = count;
	nvr.param = NULL;
	nvr.value = buf;
	nvr.is_temp = include_temp;

	ret = ioctl(nvram_fd, NVRAM_IOCTL_GET, &nvr);
	if (ret < 0) {
		saved_errno = errno;
		perror(PATH_DEV_NVRAM);
	}
	if (close(nvram_fd) < 0) {
		if (ret == 0) {
			ret = -1;
			saved_errno = errno;
		}
		perror(PATH_DEV_NVRAM);
	}
	if (ret < 0 && saved_errno)
		errno = saved_errno;
	return ret;
}


int
nvram_getall(char *buf, int count, int include_temp)
{
	int lock_fd, ret;

	lock_fd = nvram_api_lock_acquire(0);
	if (lock_fd < 0)
		return -1;
	ret = nvram_getall_unlocked(buf, count, include_temp);
	return nvram_api_lock_finish(lock_fd, ret);
}

static int
nvram_set_unlocked(const char *name, const char *value, int is_temp)
{
	int ret, nvram_fd;
	anvram_ioctl_t nvr;

	if (!name || !*name || is_temp < 0 || is_temp > 1) {
		errno = EINVAL;
		return -1;
	}

	nvram_fd = open(PATH_DEV_NVRAM, O_RDWR);
	if (nvram_fd < 0) {
		perror(PATH_DEV_NVRAM);
		return -1;
	}

	nvr.size = sizeof(nvr);
	nvr.len_param = strlen(name);
	nvr.len_value = 0;
	nvr.param = (char*)name;
	nvr.value = (char*)value;
	nvr.is_temp = is_temp;
	if (value)
		nvr.len_value = strlen(value);

	ret = ioctl(nvram_fd, NVRAM_IOCTL_SET, &nvr);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	close(nvram_fd);

	return ret;
}


static int
nvram_set_guarded(const char *name, const char *value, int is_temp)
{
	int lock_fd, ret;

	lock_fd = nvram_api_lock_acquire(1);
	if (lock_fd < 0)
		return -1;
	ret = nvram_set_unlocked(name, value, is_temp);
	return nvram_api_lock_finish(lock_fd, ret);
}

/* Preserve the historical shared-library symbol for existing callers. */
int
_nvram_set(const char *name, const char *value, int is_temp)
{
	return nvram_set_guarded(name, value, is_temp);
}

int
nvram_set(const char *name, const char *value)
{
	return _nvram_set(name, value, 0);
}

int
nvram_set_temp(const char *name, const char *value)
{
	return _nvram_set(name, value, 1);
}

int nvram_set_int(const char *name, int value)
{
	char int_str[16];
	sprintf(int_str, "%d", value);
	return _nvram_set(name, int_str, 0);
}

int nvram_set_int_temp(const char *name, int value)
{
	char int_str[16];
	sprintf(int_str, "%d", value);
	return _nvram_set(name, int_str, 1);
}

int
nvram_unset(const char *name)
{
	return _nvram_set(name, NULL, 0);
}

int
nvram_match(const char *name, char *match)
{
	const char *value = nvram_get(name);
	return (value && !strcmp(value, match));
}

int
nvram_invmatch(const char *name, char *invmatch)
{
	const char *value = nvram_get(name);
	return (value && strcmp(value, invmatch));
}

static int
nvram_commit_unlocked(void)
{
	int ret, nvram_fd;

	nvram_fd = open(PATH_DEV_NVRAM, O_RDWR);
	if (nvram_fd < 0) {
		perror(PATH_DEV_NVRAM);
		return -1;
	}

	ret = ioctl(nvram_fd, NVRAM_IOCTL_COMMIT, 0);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	close(nvram_fd);

	return ret;
}


int
nvram_commit(void)
{
	int lock_fd, ret;

	lock_fd = nvram_api_lock_acquire(1);
	if (lock_fd < 0)
		return -1;
	ret = nvram_commit_unlocked();
	return nvram_api_lock_finish(lock_fd, ret);
}

static int
nvram_clear_unlocked(void)
{
	int ret, nvram_fd;

	nvram_fd = open(PATH_DEV_NVRAM, O_RDWR);
	if (nvram_fd < 0) {
		perror(PATH_DEV_NVRAM);
		return -1;
	}

	ret = ioctl(nvram_fd, NVRAM_IOCTL_CLEAR, 0);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	close(nvram_fd);

	return ret;
}


int
nvram_clear(void)
{
	int lock_fd, ret;

	lock_fd = nvram_api_lock_acquire(1);
	if (lock_fd < 0)
		return -1;
	ret = nvram_clear_unlocked();
	return nvram_api_lock_finish(lock_fd, ret);
}

int
nvram_transaction_begin(struct nvram_transaction *transaction)
{
	int lock_fd;

	if (!transaction) {
		errno = EINVAL;
		return -1;
	}
	transaction->lock_fd = -1;
	transaction->owner_pid = 0;
	lock_fd = nvram_transaction_lock_acquire(
		1, NVRAM_TRANSACTION_LOCK_WAIT_SECONDS * 4);
	if (lock_fd < 0)
		return -1;
	transaction->lock_fd = lock_fd;
	transaction->owner_pid = getpid();
	return 0;
}

static int
nvram_transaction_valid(struct nvram_transaction *transaction)
{
	if (!transaction || transaction->lock_fd < 0) {
		errno = EINVAL;
		return 0;
	}
	if (transaction->owner_pid != getpid()) {
		errno = EPERM;
		return 0;
	}
	if (fcntl(transaction->lock_fd, F_GETFD) < 0)
		return 0;
	return 1;
}

int
nvram_transaction_end(struct nvram_transaction *transaction)
{
	int lock_fd;

	if (!transaction || transaction->lock_fd < 0) {
		errno = EINVAL;
		return -1;
	}
	lock_fd = transaction->lock_fd;
	transaction->lock_fd = -1;
	if (transaction->owner_pid != getpid()) {
		transaction->owner_pid = 0;
		(void)close(lock_fd);
		errno = EPERM;
		return -1;
	}
	transaction->owner_pid = 0;
	return nvram_transaction_lock_release(lock_fd);
}

int
nvram_transaction_getall(struct nvram_transaction *transaction,
			 char *buf, int count, int include_temp)
{
	if (!nvram_transaction_valid(transaction))
		return -1;
	return nvram_getall_unlocked(buf, count, include_temp);
}

int
nvram_transaction_set(struct nvram_transaction *transaction,
		      const char *name, const char *value, int is_temp)
{
	if (!nvram_transaction_valid(transaction))
		return -1;
	return nvram_set_unlocked(name, value, is_temp);
}

int
nvram_transaction_clear(struct nvram_transaction *transaction)
{
	if (!nvram_transaction_valid(transaction))
		return -1;
	return nvram_clear_unlocked();
}


/*
 *  * This program is free software; you can redistribute it and/or
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
#include <limits.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <nvram_linux.h>
#include <shutils.h>
#include <router_account_policy.h>

#include "disk_io_tools.h"
#include "disk_initial.h"
#include "disk_share.h"
#include "disk_share_list_policy.h"
#include "disk_share_account_nvram_policy.h"

static int nvram_commit_safe(void)
{
	char *manual_value = NULL;
	int read_status, commit_required;

	errno = 0;
	read_status = nvram_get_checked("nvram_manual", &manual_value);
	if (disk_share_account_commit_required(
		    read_status, manual_value, &commit_required) != 0)
		return -1;
	if (!commit_required)
		return 0;

	return nvram_commit();
}

static int
is_valid_account_name(const char *name)
{
	return router_account_name_is_valid(name);
}

static int
is_reserved_account_name(const char *name)
{
	char *management_name = NULL;
	int read_status;

	if (!name || !*name)
		return 1;
	if (router_account_name_is_reserved(name, NULL) ||
	    !strcmp(name, FTP_ANONYMOUS_USER))
		return 1;

	/*
	 * Do not permit a new storage identity to shadow the router's management
	 * account when a transient NVRAM error makes the cached getter look empty.
	 */
	errno = 0;
	read_status = nvram_get_checked("http_username", &management_name);
	if (read_status < 0 && errno == 0)
		errno = EIO;
	return router_account_name_is_reserved_after_read(
		name, read_status, management_name);
}

static int
is_valid_account_password(const char *password)
{
	const unsigned char *p;
	size_t len;

	if (!password || !*password)
		return 0;
	len = strlen(password);
	if (len >= 128) /* VSFTP_PASSWORD_MAX includes the terminating NUL. */
		return 0;
	for (p = (const unsigned char *)password; *p; ++p) {
		/* Newlines/control bytes can corrupt generated FTP/SMB metadata. */
		if (*p < 0x20 || *p == 0x7f)
			return 0;
	}
	return 1;
}

static int
is_valid_folder_name(const char *name)
{
	const unsigned char *p;
	size_t len;

	if (!name || !*name)
		return 0;

	len = strlen(name);
	if (len > 255 || !strcmp(name, ".") || !strcmp(name, ".."))
		return 0;

	for (p = (const unsigned char *)name; *p; ++p) {
		if (*p < 32 || *p == 127 || *p == '/' || *p == '\\' ||
		    *p == '[' || *p == ']' || *p == '\r' || *p == '\n')
			return 0;
	}

	return 1;
}

static int
disk_share_account_nvram_begin(void *context)
{
	return nvram_transaction_begin((struct nvram_transaction *)context);
}

static int
disk_share_account_nvram_get(void *context, const char *name, char **value)
{
	return nvram_transaction_get((struct nvram_transaction *)context,
	                             name, value);
}

static int
disk_share_account_nvram_end(void *context)
{
	return nvram_transaction_end((struct nvram_transaction *)context);
}

static int
disk_share_account_nvram_set(void *context, const char *name, const char *value)
{
	return nvram_transaction_set((struct nvram_transaction *)context,
	                             name, value, 0);
}

static int
disk_share_account_nvram_commit(void *context)
{
	return nvram_transaction_commit((struct nvram_transaction *)context);
}

static int
disk_share_account_name_valid(const char *name)
{
	return is_valid_account_name(name);
}

static int
disk_share_account_name_reserved(const char *name, const char *http_username)
{
	return router_account_name_is_reserved(name, http_username) ||
	       !strcmp(name, FTP_ANONYMOUS_USER);
}

static const struct disk_share_account_nvram_ops
disk_share_account_nvram_operations = {
	disk_share_account_nvram_begin,
	disk_share_account_nvram_get,
	disk_share_account_nvram_end,
	disk_share_account_name_valid,
	disk_share_account_name_reserved,
	disk_share_account_nvram_set,
	disk_share_account_nvram_commit
};

int get_account_list(int *acc_num, char ***account_list)
{
	struct nvram_transaction transaction;

	return disk_share_account_inventory_read(
		&disk_share_account_nvram_operations, &transaction,
		MAX_ACCOUNT_NUM, acc_num, account_list);
}

int get_folder_list_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)
{
	char **tmp_folder_list = NULL;
	char target[32];
	char *list_file = NULL, *list_info = NULL;
	char *follow_info, *line_end, *endptr;
	int i, count, saved_errno, valid_count = 0;
	int stat_result, stat_error, directory_status;
	long parsed_count;
	struct stat entry_status;
	char *folder_path = NULL;
	size_t list_file_len, list_info_len, name_len;
	size_t mount_len, folder_path_size;
	int target_len;

	if (disk_share_list_prepare_outputs(mount_path, sh_num, folder_list) != 0)
		return -1;

	list_file_len = strlen(mount_path) + strlen("/.__folder_list.txt");
	if (list_file_len < strlen(mount_path))
		return -1;
	list_file = (char *)malloc(list_file_len + 1);
	if (!list_file)
		return -1;

	snprintf(list_file, list_file_len + 1, "%s/.__folder_list.txt", mount_path);

	list_info = read_regular_file_nofollow(list_file);
	if (!list_info && errno == ENOENT) {
		if (initial_folder_list_in_mount_path(mount_path) != 0) {
			saved_errno = errno ? errno : EIO;
			free(list_file);
			errno = saved_errno;
			return -1;
		}
		list_info = read_regular_file_nofollow(list_file);
	}

	free(list_file);
	if (!list_info)
		return -1;

	list_info_len = strlen(list_info);
	follow_info = strstr(list_info, "sh_num=");
	if (!follow_info)
		goto invalid_list;

	follow_info += strlen("sh_num=");
	if (*follow_info < '0' || *follow_info > '9')
		goto invalid_list;

	parsed_count = strtol(follow_info, &endptr, 10);
	if (endptr == follow_info || parsed_count < 0 || parsed_count > INT_MAX)
		goto invalid_list;
	if (*endptr != '\n' && !(*endptr == '\r' && endptr[1] == '\n'))
		goto invalid_list;

	count = (int)parsed_count;
	if (count == 0) {
		free(list_info);
		return 0;
	}

	/* Revalidate cached entries without following attacker-controlled links. */
	mount_len = strlen(mount_path);
	if (mount_len > (size_t)-1 - 257) {
		errno = ENAMETOOLONG;
		goto invalid_list;
	}
	folder_path_size = mount_len + 257;
	folder_path = (char *)malloc(folder_path_size);
	if (!folder_path)
		goto invalid_list;

	/* Each entry needs at least: newline + "sh_nameN=" + newline. */
	if ((size_t)count > list_info_len / 11)
		goto invalid_list;

	tmp_folder_list = (char **)calloc((size_t)count, sizeof(char *));
	if (!tmp_folder_list)
		goto invalid_list;

	for (i = 0; i < count; ++i) {
		target_len = snprintf(target, sizeof(target), "\nsh_name%d=", i);
		if (target_len < 0 || target_len >= (int)sizeof(target))
			goto invalid_entries;

		follow_info = strstr(list_info, target);
		if (!follow_info)
			goto invalid_entries;

		follow_info += target_len;
		line_end = strchr(follow_info, '\n');
		if (!line_end)
			goto invalid_entries;

		name_len = (size_t)(line_end - follow_info);
		if (name_len == 0)
			goto invalid_entries;

		tmp_folder_list[i] = (char *)malloc(name_len + 1);
		if (!tmp_folder_list[i])
			goto invalid_entries;

		memcpy(tmp_folder_list[i], follow_info, name_len);
		tmp_folder_list[i][name_len] = '\0';
		if (!is_valid_folder_name(tmp_folder_list[i]))
			goto invalid_entries;
		if (!disk_share_folder_name_is_visible(
			    tmp_folder_list[i], 1,
			    test_if_System_folder(tmp_folder_list[i]))) {
			free(tmp_folder_list[i]);
			tmp_folder_list[i] = NULL;
			continue;
		}

		target_len = snprintf(folder_path, folder_path_size, "%s/%s",
		                     mount_path, tmp_folder_list[i]);
		if (target_len < 0 || (size_t)target_len >= folder_path_size) {
			errno = ENAMETOOLONG;
			goto invalid_entries;
		}
		stat_result = lstat(folder_path, &entry_status);
		stat_error = errno;
		directory_status = disk_share_folder_entry_status(
			stat_result, stat_error,
			stat_result == 0 ? entry_status.st_mode : (mode_t)0);
		if (directory_status < 0)
			goto invalid_entries;
		if (directory_status == 0) {
			free(tmp_folder_list[i]);
			tmp_folder_list[i] = NULL;
			continue;
		}
		if (valid_count != i) {
			tmp_folder_list[valid_count] = tmp_folder_list[i];
			tmp_folder_list[i] = NULL;
		}
		++valid_count;
	}

	free(folder_path);
	free(list_info);
	*sh_num = valid_count;
	*folder_list = tmp_folder_list;
	return 0;

invalid_entries:
	free_2_dimension_list(&count, &tmp_folder_list);
invalid_list:
	free(folder_path);
	free(list_info);
	return -1;
}
int get_all_folder_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list)
{
	DIR *pool_to_open;
	struct dirent *dp;
	char *testdir, *folder_name;
	char **tmp_folder_list = NULL, **new_folder_list;
	int count = 0, i;
	int read_status, read_error, saved_errno;
	int stat_result, stat_error, directory_status;
	struct stat entry_status;
	size_t mount_len, name_len, path_len;

	if (disk_share_list_prepare_outputs(mount_path, sh_num, folder_list) != 0)
		return -1;
	mount_len = strlen(mount_path);

	pool_to_open = opendir(mount_path);
	if (!pool_to_open)
		return -1;

	for (;;) {
		read_status = disk_share_list_next_entry(
			pool_to_open, readdir, &dp, &read_error);
		if (read_status == 0)
			break;
		if (read_status < 0) {
			errno = read_error != 0 ? read_error : EIO;
			goto failed;
		}

		if (!disk_share_folder_name_is_visible(
			    dp->d_name, is_valid_folder_name(dp->d_name),
			    test_if_System_folder(dp->d_name)))
			continue;

		name_len = strlen(dp->d_name);
		if (mount_len > (size_t)-1 - name_len - 2)
			goto failed;

		path_len = mount_len + name_len + 1;
		testdir = (char *)malloc(path_len + 1);
		if (!testdir)
			goto failed;

		snprintf(testdir, path_len + 1, "%s/%s", mount_path, dp->d_name);
		stat_result = lstat(testdir, &entry_status);
		stat_error = errno;
		directory_status = disk_share_folder_entry_status(
			stat_result, stat_error,
			stat_result == 0 ? entry_status.st_mode : (mode_t)0);
		if (directory_status == 0) {
			free(testdir);
			continue;
		}
		if (directory_status < 0) {
			saved_errno = errno ? errno : EIO;
			free(testdir);
			errno = saved_errno;
			goto failed;
		}
		free(testdir);

		if (count == INT_MAX ||
		    (size_t)count >= (size_t)-1 / sizeof(*tmp_folder_list))
			goto failed;

		folder_name = (char *)malloc(name_len + 1);
		if (!folder_name)
			goto failed;
		memcpy(folder_name, dp->d_name, name_len + 1);

		new_folder_list = (char **)realloc(tmp_folder_list,
						  sizeof(*tmp_folder_list) * (size_t)(count + 1));
		if (!new_folder_list) {
			free(folder_name);
			goto failed;
		}

		tmp_folder_list = new_folder_list;
		tmp_folder_list[count++] = folder_name;
	}

	if (closedir(pool_to_open) != 0) {
		pool_to_open = NULL;
		goto failed;
	}
	pool_to_open = NULL;
	*sh_num = count;
	*folder_list = tmp_folder_list;
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (pool_to_open)
		(void)closedir(pool_to_open);
	for (i = 0; i < count; ++i)
		free(tmp_folder_list[i]);
	free(tmp_folder_list);
	errno = saved_errno;
	return -1;
}
void free_2_dimension_list(int *num, char ***list)
{
	int i;
	char **target;

	if (!num || !list)
		return;

	target = *list;
	if (target) {
		for (i = 0; i < *num; ++i)
			if (target[i] != NULL)
				free(target[i]);
		free(target);
	}

	*num = 0;
	*list = NULL;
}

int initial_folder_list_in_mount_path(const char *const mount_path)
{
	int sh_num = 0, fd = -1, temp_created = 0;
	char **folder_list = NULL;
	FILE *fp = NULL;
	char *list_file = NULL, *temp_file = NULL;
	int result, saved_errno;
	size_t mount_len, list_len, temp_len;
	const char list_suffix[] = "/.__folder_list.txt";
	const char temp_suffix[] = ".tmp.XXXXXX";

	if (!mount_path || !*mount_path) {
		errno = EINVAL;
		return -1;
	}

	result = get_all_folder_in_mount_path(mount_path, &sh_num, &folder_list);
	if (result != 0) {
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}

	mount_len = strlen(mount_path);
	if (mount_len > (size_t)-1 - (sizeof(list_suffix) - 1) -
	                (sizeof(temp_suffix) - 1) - 1) {
		errno = ENAMETOOLONG;
		goto failed;
	}
	list_len = mount_len + sizeof(list_suffix) - 1;
	temp_len = list_len + sizeof(temp_suffix) - 1;
	list_file = (char *)malloc(list_len + 1);
	temp_file = (char *)malloc(temp_len + 1);
	if (!list_file || !temp_file)
		goto failed;

	memcpy(list_file, mount_path, mount_len);
	memcpy(list_file + mount_len, list_suffix, sizeof(list_suffix));
	memcpy(temp_file, list_file, list_len);
	memcpy(temp_file + list_len, temp_suffix, sizeof(temp_suffix));

	fd = mkstemp(temp_file);
	if (fd < 0)
		goto failed;
	temp_created = 1;
	fp = fdopen(fd, "w");
	if (!fp)
		goto failed;
	fd = -1;

	if (disk_share_list_write_snapshot(fp, sh_num, folder_list) != 0)
		goto failed;
	if (fsync(fileno(fp)) != 0)
		goto failed;
	if (fclose(fp) != 0) {
		fp = NULL;
		goto failed;
	}
	fp = NULL;

	if (rename(temp_file, list_file) != 0)
		goto failed;
	temp_created = 0;

	free(temp_file);
	free(list_file);
	free_2_dimension_list(&sh_num, &folder_list);
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (fp)
		(void)fclose(fp);
	else if (fd >= 0)
		(void)close(fd);
	if (temp_created)
		(void)unlink(temp_file);
	free(temp_file);
	free(list_file);
	free_2_dimension_list(&sh_num, &folder_list);
	errno = saved_errno;
	return -1;
}

static int initial_one_var_file_in_mount_path_locked(const char *const account,
                                      const char *const mount_path)
{
	int sh_num = 0, fd = -1, temp_created = 0;
	char **folder_list = NULL;
	FILE *fp = NULL;
	char *var_file = NULL, *temp_file = NULL;
	int result, saved_errno;
	size_t mount_len, account_len, var_len, temp_len, base_len;
	const char prefix[] = "/.__";
	const char suffix[] = "_var.txt";
	const char temp_suffix[] = ".tmp.XXXXXX";
	int samba_right, ftp_right;

	if (!is_valid_account_name(account) || !mount_path || !*mount_path) {
		errno = EINVAL;
		return -1;
	}

	result = get_all_folder_in_mount_path(mount_path, &sh_num, &folder_list);
	if (result != 0) {
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}

	if (!strcmp(account, FTP_ANONYMOUS_USER)) {
		samba_right = 0;
		ftp_right = 1;
	} else {
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
	}

	mount_len = strlen(mount_path);
	account_len = strlen(account);
	if (mount_len > (size_t)-1 - (sizeof(prefix) - 1))
		goto path_too_long;
	base_len = mount_len + sizeof(prefix) - 1;
	if (account_len > (size_t)-1 - base_len)
		goto path_too_long;
	base_len += account_len;
	if (sizeof(suffix) - 1 > (size_t)-1 - base_len)
		goto path_too_long;
	var_len = base_len + sizeof(suffix) - 1;
	if (var_len == (size_t)-1 ||
	    sizeof(temp_suffix) - 1 > (size_t)-1 - var_len - 1)
		goto path_too_long;
	temp_len = var_len + sizeof(temp_suffix) - 1;

	var_file = (char *)malloc(var_len + 1);
	temp_file = (char *)malloc(temp_len + 1);
	if (!var_file || !temp_file)
		goto failed;

	memcpy(var_file, mount_path, mount_len);
	memcpy(var_file + mount_len, prefix, sizeof(prefix) - 1);
	memcpy(var_file + base_len - account_len, account, account_len);
	memcpy(var_file + var_len - (sizeof(suffix) - 1), suffix,
	       sizeof(suffix));
	memcpy(temp_file, var_file, var_len);
	memcpy(temp_file + var_len, temp_suffix, sizeof(temp_suffix));

	fd = mkstemp(temp_file);
	if (fd < 0)
		goto failed;
	temp_created = 1;
	fp = fdopen(fd, "w");
	if (!fp)
		goto failed;
	fd = -1;

	if (disk_share_write_account_defaults(fp, sh_num, folder_list,
	                                      samba_right, ftp_right,
	                                      DEFAULT_DMS_RIGHT) != 0)
		goto failed;
	if (fsync(fileno(fp)) != 0)
		goto failed;
	if (fclose(fp) != 0) {
		fp = NULL;
		goto failed;
	}
	fp = NULL;

	if (rename(temp_file, var_file) != 0)
		goto failed;
	temp_created = 0;

	free(temp_file);
	free(var_file);
	free_2_dimension_list(&sh_num, &folder_list);
	return 0;

path_too_long:
	errno = ENAMETOOLONG;
failed:
	saved_errno = errno ? errno : EIO;
	if (fp)
		(void)fclose(fp);
	else if (fd >= 0)
		(void)close(fd);
	if (temp_created)
		(void)unlink(temp_file);
	free(temp_file);
	free(var_file);
	free_2_dimension_list(&sh_num, &folder_list);
	errno = saved_errno;
	return -1;
}

static int
remove_stale_share_var_files(const char *mount_path,
                             int acc_num, char *const *account_list)
{
	DIR *dir;
	struct dirent *entry;
	char account[32 + 1], *path;
	int read_status, read_error, stale, saved_errno, i;
	size_t mount_len, name_len, account_len, path_len;

	if (!mount_path || !*mount_path || acc_num < 0 ||
	    acc_num > MAX_ACCOUNT_NUM || (acc_num > 0 && !account_list)) {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i < acc_num; ++i) {
		if (!account_list[i] || !is_valid_account_name(account_list[i])) {
			errno = EINVAL;
			return -1;
		}
	}

	dir = opendir(mount_path);
	if (!dir)
		return -1;
	mount_len = strlen(mount_path);

	for (;;) {
		read_status = disk_share_list_next_entry(dir, readdir,
		                                         &entry, &read_error);
		if (read_status == 0)
			break;
		if (read_status < 0) {
			errno = read_error ? read_error : EIO;
			goto failed;
		}

		name_len = strlen(entry->d_name);
		if (name_len <= 3 + 8 ||
		    strncmp(entry->d_name, ".__", 3) != 0 ||
		    strcmp(entry->d_name + name_len - 8, "_var.txt") != 0)
			continue;

		account_len = name_len - 3 - 8;
		if (account_len == 0 || account_len > 31)
			continue;
		memcpy(account, entry->d_name + 3, account_len);
		account[account_len] = '\0';
		if (!is_valid_account_name(account) ||
		    strcmp(account, FTP_ANONYMOUS_USER) == 0)
			continue;

		stale = 1;
		for (i = 0; i < acc_num; ++i) {
			if (strcmp(account_list[i], account) == 0) {
				stale = 0;
				break;
			}
		}
		if (!stale)
			continue;

		if (mount_len > (size_t)-1 - name_len - 2) {
			errno = ENAMETOOLONG;
			goto failed;
		}
		path_len = mount_len + 1 + name_len;
		path = (char *)malloc(path_len + 1);
		if (!path)
			goto failed;
		memcpy(path, mount_path, mount_len);
		path[mount_len] = '/';
		memcpy(path + mount_len + 1, entry->d_name, name_len + 1);
		if (unlink(path) != 0 && errno != ENOENT) {
			saved_errno = errno ? errno : EIO;
			free(path);
			errno = saved_errno;
			goto failed;
		}
		free(path);
	}

	if (closedir(dir) != 0) {
		dir = NULL;
		goto failed;
	}
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (dir)
		(void)closedir(dir);
	errno = saved_errno;
	return -1;
}
static int
disk_share_account_metadata_write(
	const char *const account, const char *const mount_path,
	int (*writer)(const char *const, const char *const))
{
	int acc_num = 0, account_found = 0, transaction_started = 0;
	int i, result, first_error = 0, end_error;
	char **account_list = NULL;
	struct nvram_transaction transaction;

	if (!account || !is_valid_account_name(account) ||
	    !mount_path || !*mount_path || !writer) {
		errno = EINVAL;
		return -1;
	}
	if (get_account_list(&acc_num, &account_list) != 0) {
		first_error = errno ? errno : EIO;
		goto finished;
	}
	if (acc_num < 0 || acc_num > MAX_ACCOUNT_NUM ||
	    (acc_num > 0 && !account_list)) {
		first_error = EINVAL;
		goto finished;
	}

	if (strcmp(account, FTP_ANONYMOUS_USER) == 0)
		account_found = 1;
	else {
		for (i = 0; i < acc_num; ++i) {
			if (account_list[i] && strcmp(account_list[i], account) == 0) {
				account_found = 1;
				break;
			}
		}
	}
	if (!account_found) {
		first_error = ENOENT;
		goto finished;
	}

	if (disk_share_account_nvram_begin(&transaction) != 0) {
		first_error = errno ? errno : EIO;
		goto finished;
	}
	transaction_started = 1;
	if (disk_share_account_inventory_matches_locked(
		    &disk_share_account_nvram_operations, &transaction,
		    acc_num, MAX_ACCOUNT_NUM, account_list) != 0) {
		first_error = errno ? errno : EIO;
		goto release;
	}

	errno = 0;
	result = writer(account, mount_path);
	if (result != 0)
		first_error = errno ? errno : EIO;

release:
	errno = 0;
	if (transaction_started && disk_share_account_nvram_end(&transaction) != 0) {
		end_error = errno ? errno : EIO;
		if (first_error == 0)
			first_error = end_error;
		else
			logmessage("disk_share",
				   "account metadata transaction release failed (errno %d)",
				   end_error);
	}
finished:
	free_2_dimension_list(&acc_num, &account_list);
	if (first_error != 0) {
		errno = first_error;
		return -1;
	}
	return 0;
}

int initial_one_var_file_in_mount_path(const char *const account,
                                   const char *const mount_path)
{
	return disk_share_account_metadata_write(
		account, mount_path, initial_one_var_file_in_mount_path_locked);
}

int initial_all_var_file_in_mount_path(const char *const mount_path)
{
	int i, first_error = 0, end_error;
	int acc_num = 0, transaction_started = 0;
	char **account_list = NULL;
	char var_file[256];
	struct nvram_transaction transaction;

	if (!mount_path || !*mount_path) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * Read and preflight the complete identity set before changing any file.
	 * Rebuild active metadata atomically first, then prune orphaned account
	 * files only after every active file and the folder-list snapshot succeed.
	 * A failed rebuild therefore never starts by deleting working ACLs.
	 */
	if (get_account_list(&acc_num, &account_list) != 0) {
		first_error = errno ? errno : EIO;
		goto finished;
	}
	if (disk_share_format_account_var_path(var_file, sizeof(var_file),
	                                       mount_path, FTP_ANONYMOUS_USER) != 0) {
		first_error = errno ? errno : EIO;
		goto finished;
	}
	for (i = 0; i < acc_num; ++i) {
		if (disk_share_format_account_var_path(var_file, sizeof(var_file),
		                                       mount_path, account_list[i]) != 0) {
			first_error = errno ? errno : EIO;
			goto finished;
		}
	}

	if (disk_share_account_nvram_begin(&transaction) != 0) {
		first_error = errno ? errno : EIO;
		goto finished;
	}
	transaction_started = 1;
	if (disk_share_account_inventory_matches_locked(
		    &disk_share_account_nvram_operations, &transaction,
		    acc_num, MAX_ACCOUNT_NUM, account_list) != 0) {
		first_error = errno ? errno : EIO;
		goto finished;
	}

	/* Initialize anonymous defaults before account-specific permissions. */
	if (initial_one_var_file_in_mount_path_locked(FTP_ANONYMOUS_USER,
	                                       mount_path) != 0) {
		first_error = errno ? errno : EIO;
		goto finished;
	}

	for (i = 0; i < acc_num; ++i) {
		if (initial_one_var_file_in_mount_path_locked(account_list[i],
		                                       mount_path) != 0) {
			first_error = errno ? errno : EIO;
			goto finished;
		}
	}

	/* Publish the folder-list snapshot only after all account files exist. */
	if (initial_folder_list_in_mount_path(mount_path) != 0) {
		first_error = errno ? errno : EIO;
		goto finished;
	}

	/*
	 * All active files are now durable. Remove only valid, unreferenced user
	 * files; never delete the just-rebuilt active or anonymous ACLs.
	 */
	if (remove_stale_share_var_files(mount_path, acc_num, account_list) != 0)
		first_error = errno ? errno : EIO;

finished:
	free_2_dimension_list(&acc_num, &account_list);
	if (transaction_started) {
		errno = 0;
		if (disk_share_account_nvram_end(&transaction) != 0) {
			end_error = errno ? errno : EIO;
			if (first_error == 0)
				first_error = end_error;
			else
				logmessage("disk_share",
					   "full metadata rebuild transaction release failed (errno %d)",
					   end_error);
		}
	}
	if (first_error != 0) {
		errno = first_error;
		return -1;
	}
	return 0;
}


static int modify_if_exist_new_folder_locked(
	const char *const account, const char *const mount_path);

static int
create_if_no_var_files_with_accounts(const char *const mount_path,
                                     int acc_num, char **account_list)
{
	int i, path_len, result, file_status;
	char var_file[256];

	/*
	 * Reject invalid snapshots and preflight all account paths before the
	 * anonymous permission file or any named account metadata is changed.
	 */
	if (!mount_path || !*mount_path || acc_num < 0 ||
	    acc_num > MAX_ACCOUNT_NUM || (acc_num > 0 && !account_list)) {
		errno = EINVAL;
		return -1;
	}

	for (i = 0; i < acc_num; ++i) {
		if (!account_list[i] || !*account_list[i]) {
			errno = EINVAL;
			return -1;
		}
		path_len = snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt",
		                    mount_path, account_list[i]);
		if (path_len < 0 || path_len >= (int)sizeof(var_file)) {
			errno = ENAMETOOLONG;
			return -1;
		}
	}

	path_len = snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt",
	                    mount_path, FTP_ANONYMOUS_USER);
	if (path_len < 0 || path_len >= (int)sizeof(var_file)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	file_status = test_if_regular_file_nofollow(var_file);
	if (file_status < 0)
		return -1;
	if (file_status == 0)
		result = initial_one_var_file_in_mount_path_locked(FTP_ANONYMOUS_USER,
		                                           mount_path);
	else
		result = modify_if_exist_new_folder_locked(FTP_ANONYMOUS_USER, mount_path);
	if (result != 0)
		return -1;

	for (i = 0; i < acc_num; ++i) {
		path_len = snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt",
		                    mount_path, account_list[i]);
		if (path_len < 0 || path_len >= (int)sizeof(var_file)) {
			errno = ENAMETOOLONG;
			result = -1;
			break;
		}
		file_status = test_if_regular_file_nofollow(var_file);
		if (file_status < 0) {
			result = -1;
			break;
		}
		if (file_status == 0)
			result = initial_one_var_file_in_mount_path_locked(account_list[i],
			                                           mount_path);
		else
			result = modify_if_exist_new_folder_locked(account_list[i], mount_path);
		if (result != 0)
			break;
	}
	if (result != 0)
		return -1;
	return 0;
}

int
create_if_no_var_files(const char *const mount_path)
{
	int acc_num = 0, result, saved_errno = 0, end_errno;
	char **account_list = NULL;
	struct nvram_transaction transaction;

	if (!mount_path || !*mount_path) {
		errno = EINVAL;
		return -1;
	}
	if (get_account_list(&acc_num, &account_list) != 0) {
		saved_errno = errno ? errno : EIO;
		free_2_dimension_list(&acc_num, &account_list);
		errno = saved_errno;
		return -1;
	}

	if (disk_share_account_nvram_begin(&transaction) != 0) {
		saved_errno = errno ? errno : EIO;
		goto finished;
	}
	if (disk_share_account_inventory_matches_locked(
		    &disk_share_account_nvram_operations, &transaction,
		    acc_num, MAX_ACCOUNT_NUM, account_list) != 0) {
		saved_errno = errno ? errno : EIO;
		goto release;
	}

	result = create_if_no_var_files_with_accounts(
		mount_path, acc_num, account_list);
	if (result != 0)
		saved_errno = errno ? errno : EIO;

release:
	errno = 0;
	if (disk_share_account_nvram_end(&transaction) != 0) {
		end_errno = errno ? errno : EIO;
		if (saved_errno == 0)
			saved_errno = end_errno;
		else
			logmessage("disk_share",
				   "metadata initialization transaction release failed (errno %d)",
				   end_errno);
	}

finished:
	free_2_dimension_list(&acc_num, &account_list);
	if (saved_errno != 0) {
		errno = saved_errno;
		return -1;
	}
	return 0;
}


static int modify_if_exist_new_folder_locked(const char *const account,
                                  const char *const mount_path)
{
	int sh_num = 0, missing_count = 0, fd = -1, temp_created = 0;
	char **folder_list = NULL, **missing_folders = NULL;
	char *old_info = NULL, *var_file = NULL, *temp_file = NULL;
	FILE *fp = NULL;
	int result, i, samba_right, ftp_right, saved_errno;
	size_t mount_len, account_len, base_len, var_len, temp_len, old_len;
	const char prefix[] = "/.__";
	const char suffix[] = "_var.txt";
	const char temp_suffix[] = ".tmp.XXXXXX";

	if (!is_valid_account_name(account) || !mount_path || !*mount_path) {
		errno = EINVAL;
		return -1;
	}

	result = get_all_folder_in_mount_path(mount_path, &sh_num, &folder_list);
	if (result != 0) {
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}

	if (sh_num > 0) {
		if ((size_t)sh_num > (size_t)-1 / sizeof(*missing_folders)) {
			errno = EOVERFLOW;
			goto failed;
		}
		missing_folders = (char **)malloc((size_t)sh_num *
		                                  sizeof(*missing_folders));
		if (!missing_folders)
			goto failed;
	}

	for (i = 0; i < sh_num; ++i) {
		result = test_if_exist_folder_in_mount_path(mount_path, folder_list[i]);
		if (result < 0)
			goto failed;
		if (result == 0)
			missing_folders[missing_count++] = folder_list[i];
	}

	if (missing_count == 0) {
		free(missing_folders);
		free_2_dimension_list(&sh_num, &folder_list);
		return 0;
	}

	if (!strcmp(account, FTP_ANONYMOUS_USER)) {
		samba_right = 1;
		ftp_right = 1;
	} else {
		samba_right = DEFAULT_SAMBA_RIGHT;
		ftp_right = DEFAULT_FTP_RIGHT;
	}

	mount_len = strlen(mount_path);
	account_len = strlen(account);
	if (mount_len > (size_t)-1 - (sizeof(prefix) - 1))
		goto path_too_long;
	base_len = mount_len + sizeof(prefix) - 1;
	if (account_len > (size_t)-1 - base_len)
		goto path_too_long;
	base_len += account_len;
	if (sizeof(suffix) - 1 > (size_t)-1 - base_len)
		goto path_too_long;
	var_len = base_len + sizeof(suffix) - 1;
	if (var_len == (size_t)-1 ||
	    sizeof(temp_suffix) - 1 > (size_t)-1 - var_len - 1)
		goto path_too_long;
	temp_len = var_len + sizeof(temp_suffix) - 1;

	var_file = (char *)malloc(var_len + 1);
	temp_file = (char *)malloc(temp_len + 1);
	if (!var_file || !temp_file)
		goto failed;

	memcpy(var_file, mount_path, mount_len);
	memcpy(var_file + mount_len, prefix, sizeof(prefix) - 1);
	memcpy(var_file + base_len - account_len, account, account_len);
	memcpy(var_file + var_len - (sizeof(suffix) - 1), suffix,
	       sizeof(suffix));
	memcpy(temp_file, var_file, var_len);
	memcpy(temp_file + var_len, temp_suffix, sizeof(temp_suffix));

	old_info = read_regular_file_nofollow(var_file);
	if (!old_info)
		goto failed;
	old_len = strlen(old_info);

	fd = mkstemp(temp_file);
	if (fd < 0)
		goto failed;
	temp_created = 1;
	fp = fdopen(fd, "w");
	if (!fp)
		goto failed;
	fd = -1;

	if (old_len > 0 && fwrite(old_info, 1, old_len, fp) != old_len)
		goto failed;
	if (old_len > 0 && old_info[old_len - 1] != '\n' &&
	    fputc('\n', fp) == EOF)
		goto failed;
	if (disk_share_write_account_defaults(fp, missing_count,
	                                      missing_folders, samba_right,
	                                      ftp_right, DEFAULT_DMS_RIGHT) != 0)
		goto failed;
	if (fsync(fileno(fp)) != 0)
		goto failed;
	if (fclose(fp) != 0) {
		fp = NULL;
		goto failed;
	}
	fp = NULL;

	if (rename(temp_file, var_file) != 0)
		goto failed;
	temp_created = 0;

	free(old_info);
	free(temp_file);
	free(var_file);
	free(missing_folders);
	free_2_dimension_list(&sh_num, &folder_list);
	return 0;

path_too_long:
	errno = ENAMETOOLONG;
failed:
	saved_errno = errno ? errno : EIO;
	if (fp)
		(void)fclose(fp);
	else if (fd >= 0)
		(void)close(fd);
	if (temp_created)
		(void)unlink(temp_file);
	free(old_info);
	free(temp_file);
	free(var_file);
	free(missing_folders);
	free_2_dimension_list(&sh_num, &folder_list);
	errno = saved_errno;
	return -1;
}

int modify_if_exist_new_folder(const char *const account,
                           const char *const mount_path)
{
	return disk_share_account_metadata_write(
		account, mount_path, modify_if_exist_new_folder_locked);
}


int get_permission(const char *account,
		   const char *mount_path,
		   const char *folder,
		   const char *protocol)
{
char var_file[256];
	char *target, *follow_info, *var_info;
	int len, result, saved_errno;

	if (!mount_path || !*mount_path || !is_valid_account_name(account) ||
	    !is_valid_folder_name(folder) || !protocol)
		return -1;

	/* An absent metadata file means no explicit grant; read failures do not. */
	if (disk_share_format_account_var_path(var_file, sizeof(var_file),
	                                     mount_path, account) != 0)
		return -1;
	var_info = read_regular_file_nofollow(var_file);
	if (!var_info) {
		saved_errno = errno ? errno : EIO;
		if (saved_errno == ENOENT)
			return 0;
		errno = saved_errno;
		return -1;
	}

	len = strlen(folder) + strlen("*=");
	target = (char *)malloc(sizeof(char) * (len + 1));
	if (!target) {
		saved_errno = errno ? errno : ENOMEM;
		free(var_info);
		errno = saved_errno;
		return -1;
	}
	sprintf(target, "*%s=", folder);
	target[len] = 0;

	follow_info = upper_strstr(var_info, target);
	free(target);
	if (follow_info == NULL) {
		free(var_info);
		return -1;
	}

	follow_info += len;
	if (strlen(follow_info) < 4 || follow_info[3] != '\n') {
		free(var_info);
		return -1;
	}

	if (!strcmp(protocol, "cifs"))
		result = follow_info[0] - '0';
	else if (!strcmp(protocol, "ftp"))
		result = follow_info[1] - '0';
	else if (!strcmp(protocol, "dms"))
		result = follow_info[2] - '0';
	else {
		free(var_info);
		return -1;
	}
	free(var_info);

	if (result < 0 || result > 3)
		return -1;

	return result;
}


static int
disk_share_replace_file_atomic(const char *path, const char *contents,
                               size_t contents_len)
{
	const char suffix[] = ".tmp.XXXXXX";
	char *temp_path = NULL;
	struct stat existing_stat;
	FILE *stream = NULL;
	size_t path_len;
	int fd = -1, temp_created = 0, have_existing = 0, saved_errno;

	if (!path || !*path || (!contents && contents_len != 0)) {
		errno = EINVAL;
		return -1;
	}
	path_len = strlen(path);
	if (path_len > (size_t)-1 - sizeof(suffix)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	temp_path = (char *)malloc(path_len + sizeof(suffix));
	if (!temp_path)
		return -1;
	memcpy(temp_path, path, path_len);
	memcpy(temp_path + path_len, suffix, sizeof(suffix));

	if (lstat(path, &existing_stat) == 0) {
		have_existing = 1;
		if (!S_ISREG(existing_stat.st_mode)) {
			errno = EINVAL;
			goto failed;
		}
	} else if (errno != ENOENT) {
		goto failed;
	}

	fd = mkstemp(temp_path);
	if (fd < 0)
		goto failed;
	temp_created = 1;
	if (have_existing && fchmod(fd, existing_stat.st_mode & 0777) != 0)
		goto failed;

	stream = fdopen(fd, "w");
	if (!stream)
		goto failed;
	fd = -1;

	if (contents_len > 0 &&
	    fwrite(contents, 1, contents_len, stream) != contents_len)
		goto failed;
	if (fflush(stream) != 0)
		goto failed;
	if (fsync(fileno(stream)) != 0)
		goto failed;
	if (fclose(stream) != 0) {
		stream = NULL;
		goto failed;
	}
	stream = NULL;

	if (rename(temp_path, path) != 0)
		goto failed;
	temp_created = 0;

	free(temp_path);
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (stream)
		(void)fclose(stream);
	else if (fd >= 0)
		(void)close(fd);
	if (temp_created)
		(void)unlink(temp_path);
	free(temp_path);
	errno = saved_errno;
	return -1;
}


int set_permission(const char *account,
		   const char *mount_path,
		   const char *folder,
		   const char *protocol,
		   const int flag)
{
	char var_file[256];
	char *target = NULL, *follow_info, *var_info = NULL, *updated_info = NULL;
	char **account_list = NULL;
	struct nvram_transaction transaction;
	int acc_num = 0, transaction_started = 0, account_found = 0;
	int len, result, saved_errno, samba_right = 0, ftp_right = 0;
	int folder_stat_result, folder_stat_error, directory_status;
	struct stat folder_entry_stat;
	int dms_right = DEFAULT_DMS_RIGHT;
	size_t old_len, target_len, updated_len, separator_len;
	size_t mount_len, folder_len, folder_path_len;
	char *folder_path = NULL;
	char record_rights[5];

	if (flag < 0 || flag > 3 || !protocol ||
	    (strcmp(protocol, "cifs") && strcmp(protocol, "ftp") &&
	     strcmp(protocol, "dms")) ||
	    !mount_path || !*mount_path || !is_valid_account_name(account) ||
	    !is_valid_folder_name(folder)) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * Permission updates share the account writer lease with account/folder
	 * lifecycle operations. Without it, a stale read-modify-write can restore
	 * a deleted folder row or overwrite a concurrent rename/add operation.
	 */
	if (get_account_list(&acc_num, &account_list) != 0) {
		saved_errno = errno ? errno : EIO;
		free_2_dimension_list(&acc_num, &account_list);
		errno = saved_errno;
		return -1;
	}
	if (strcmp(account, FTP_ANONYMOUS_USER) == 0) {
		account_found = 1;
	} else {
		int i;

		for (i = 0; i < acc_num; ++i) {
			if (account_list[i] && strcmp(account_list[i], account) == 0) {
				account_found = 1;
				break;
			}
		}
	}
	if (!account_found) {
		saved_errno = ENOENT;
		free_2_dimension_list(&acc_num, &account_list);
		errno = saved_errno;
		return -1;
	}

	if (disk_share_account_nvram_begin(&transaction) != 0)
		goto failed;
	transaction_started = 1;
	if (disk_share_account_inventory_matches_locked(
		    &disk_share_account_nvram_operations, &transaction,
		    acc_num, MAX_ACCOUNT_NUM, account_list) != 0)
		goto failed;

	/*
	 * Validate both the live directory and its cached folder-list entry while
	 * holding the writer lease. A stale permission row must not let callers
	 * recreate access metadata for a folder that has already been removed.
	 */
	mount_len = strlen(mount_path);
	folder_len = strlen(folder);
	if (folder_len > (size_t)-1 - 2 ||
	    mount_len > (size_t)-1 - folder_len - 2) {
		errno = ENAMETOOLONG;
		goto failed;
	}
	folder_path_len = mount_len + 1 + folder_len;
	folder_path = (char *)malloc(folder_path_len + 1);
	if (!folder_path)
		goto failed;
	memcpy(folder_path, mount_path, mount_len);
	folder_path[mount_len] = '/';
	memcpy(folder_path + mount_len + 1, folder, folder_len + 1);
	folder_stat_result = lstat(folder_path, &folder_entry_stat);
	folder_stat_error = errno;
	directory_status = disk_share_folder_entry_status(
		folder_stat_result, folder_stat_error,
		folder_stat_result == 0 ? folder_entry_stat.st_mode : (mode_t)0);
	if (directory_status != 1) {
		if (folder_stat_result != 0 && folder_stat_error == ENOENT)
			errno = ENOENT;
		else if (directory_status == 0)
			errno = ENOTDIR;
		goto failed;
	}
	result = test_if_exist_folder_in_mount_path(mount_path, folder);
	if (result < 0)
		goto failed;
	if (result == 0) {
		if (initial_folder_list_in_mount_path(mount_path) != 0)
			goto failed;
		result = test_if_exist_folder_in_mount_path(mount_path, folder);
		if (result < 0)
			goto failed;
		if (result == 0) {
			errno = ENOENT;
			goto failed;
		}
	}

	if (disk_share_format_account_var_path(var_file, sizeof(var_file),
	                                       mount_path, account) != 0)
		goto failed;
	var_info = read_regular_file_nofollow(var_file);
	if (!var_info) {
		saved_errno = errno ? errno : EIO;
		if (saved_errno != ENOENT) {
			errno = saved_errno;
			goto failed;
		}
		errno = 0;
		result = initial_one_var_file_in_mount_path_locked(account, mount_path);
		if (result != 0) {
			if (!errno)
				errno = EIO;
			goto failed;
		}
		var_info = read_regular_file_nofollow(var_file);
		if (!var_info) {
			if (!errno)
				errno = EIO;
			goto failed;
		}
	}

	len = (int)(strlen("*") + strlen(folder) + strlen("="));
	target = (char *)malloc((size_t)len + 1);
	if (!target)
		goto failed;
	sprintf(target, "*%s=", folder);
	target[len] = '\0';

	follow_info = upper_strstr(var_info, target);
	if (!follow_info) {
		result = initial_folder_list_in_mount_path(mount_path);
		if (result != 0)
			goto failed;

		if (!strcmp(protocol, "cifs"))
			samba_right = flag;
		else if (!strcmp(protocol, "ftp"))
			ftp_right = flag;
		else {
			samba_right = 0;
			ftp_right = 0;
			dms_right = flag;
		}
		record_rights[0] = (char)('0' + samba_right);
		record_rights[1] = (char)('0' + ftp_right);
		record_rights[2] = (char)('0' + dms_right);
		record_rights[3] = '\n';
		record_rights[4] = '\0';

		old_len = strlen(var_info);
		target_len = strlen(target);
		separator_len = (old_len > 0 && var_info[old_len - 1] != '\n') ? 1 : 0;
		if (old_len > (size_t)-1 - separator_len ||
		    old_len + separator_len > (size_t)-1 - target_len ||
		    old_len + separator_len + target_len > (size_t)-1 - 4 ||
		    old_len + separator_len + target_len + 4 == (size_t)-1) {
			errno = EOVERFLOW;
			goto failed;
		}
		updated_len = old_len + separator_len + target_len + 4;
		updated_info = (char *)malloc(updated_len + 1);
		if (!updated_info)
			goto failed;
		if (old_len)
			memcpy(updated_info, var_info, old_len);
		if (separator_len)
			updated_info[old_len] = '\n';
		memcpy(updated_info + old_len + separator_len, target, target_len);
		memcpy(updated_info + old_len + separator_len + target_len,
		       record_rights, 4);
		updated_info[updated_len] = '\0';

		if (disk_share_replace_file_atomic(var_file, updated_info,
		                                   updated_len) != 0)
			goto failed;
	} else {
		follow_info += len;
		if (strlen(follow_info) < 4 || follow_info[3] != '\n') {
			errno = EINVAL;
			goto failed;
		}

		if (!strcmp(protocol, "cifs")) {
			if (follow_info[0] == '0' + flag)
				goto unchanged;
			follow_info[0] = (char)('0' + flag);
		} else if (!strcmp(protocol, "ftp")) {
			if (follow_info[1] == '0' + flag)
				goto unchanged;
			follow_info[1] = (char)('0' + flag);
		} else {
			if (follow_info[2] == '0' + flag)
				goto unchanged;
			follow_info[2] = (char)('0' + flag);
		}

		updated_len = strlen(var_info);
		if (disk_share_replace_file_atomic(var_file, var_info,
		                                   updated_len) != 0)
			goto failed;
	}

	free(target);
	free(var_info);
	free(updated_info);
	free(folder_path);
	free_2_dimension_list(&acc_num, &account_list);
	errno = 0;
	if (disk_share_account_nvram_end(&transaction) != 0) {
		errno = errno ? errno : EIO;
		return -1;
	}
	transaction_started = 0;

	if (!strcmp(protocol, "cifs"))
		system("/sbin/run_samba");
	return 0;

unchanged:
	free(target);
	free(var_info);
	free(folder_path);
	free_2_dimension_list(&acc_num, &account_list);
	errno = 0;
	if (disk_share_account_nvram_end(&transaction) != 0) {
		errno = errno ? errno : EIO;
		return -1;
	}
	transaction_started = 0;
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	free(target);
	free(var_info);
	free(updated_info);
	free(folder_path);
	free_2_dimension_list(&acc_num, &account_list);
	if (transaction_started) {
		errno = 0;
		if (disk_share_account_nvram_end(&transaction) != 0)
			logmessage("disk_share",
				   "permission update transaction release failed (errno %d)",
				   errno ? errno : EIO);
	}
	errno = saved_errno;
	return -1;
}

int add_account(const char *const account, const char *const password)
{
	disk_info_t *disk_list = NULL, *follow_disk;
	partition_info_t *follow_partition;
	int i, acc_num = 0, first_error = 0, init_result;
	int service_result, service_error, rollback_error = 0;
	int transaction_started = 0;
	struct nvram_transaction transaction;
	char **account_list = NULL;
	struct stat metadata_stat;
	struct disk_share_account_created_file_entry *files = NULL, *files_tail = NULL;
	struct disk_share_account_created_file_entry *file, *next_file;
	char var_file[256];

	if (!is_valid_account_name(account) || is_reserved_account_name(account))
		return -1;
	if (!is_valid_account_password(password))
		return -1;

	/* 1. Check the current account inventory before staging any file. */
	if (get_account_list(&acc_num, &account_list) != 0)
		return -1;
	if (acc_num >= MAX_ACCOUNT_NUM) {
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}
	for (i = 0; i < acc_num; ++i) {
		if (!strcmp(account, account_list[i])) {
			free_2_dimension_list(&acc_num, &account_list);
			return -1;
		}
	}

	/*
	 * Build and preflight the complete metadata plan first. An incomplete
	 * mount inventory or an existing account file must not publish a new
	 * NVRAM account whose volume permissions cannot be initialized.
	 */
	if (read_disk_data_checked(&disk_list) != 0) {
		first_error = errno ? errno : EIO;
	} else {
		for (follow_disk = disk_list;
		     follow_disk != NULL && first_error == 0;
		     follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions;
			     follow_partition != NULL && first_error == 0;
			     follow_partition = follow_partition->next) {
				if (!follow_partition->mount_point)
					continue;
				if (disk_share_format_account_var_path(
					    var_file, sizeof(var_file),
					    follow_partition->mount_point, account) != 0) {
					first_error = errno ? errno : EIO;
					break;
				}
				if (lstat(var_file, &metadata_stat) == 0) {
					first_error = EEXIST;
					break;
				}
				if (errno != ENOENT) {
					first_error = errno ? errno : EIO;
					break;
				}
				if (strlen(follow_partition->mount_point) >=
				    sizeof(((struct disk_share_account_created_file_entry *)0)->mount_path)) {
					first_error = ENAMETOOLONG;
					break;
				}
				file = (struct disk_share_account_created_file_entry *)
					calloc(1, sizeof(*file));
				if (!file) {
					first_error = errno ? errno : ENOMEM;
					break;
				}
				strcpy(file->path, var_file);
				strcpy(file->mount_path, follow_partition->mount_point);
				file->previous = files_tail;
				if (files_tail)
					files_tail->next = file;
				else
					files = file;
				files_tail = file;
			}
		}
	}

	/*
	 * Revalidate the inventory under the writer lease before creating files.
	 * Keep the lease through publication so a concurrent add cannot replace
	 * or roll back this account's per-volume metadata.
	 */
	if (first_error == 0) {
		if (disk_share_account_nvram_begin(&transaction) != 0) {
			first_error = errno ? errno : EIO;
		} else {
			transaction_started = 1;
			if (disk_share_account_inventory_add_validate_locked(
				    &disk_share_account_nvram_operations, &transaction,
				    acc_num, MAX_ACCOUNT_NUM, account_list, account) != 0)
				first_error = errno ? errno : EIO;
		}
	}

	/* Create metadata on every volume before exposing the account in NVRAM. */
	if (first_error == 0) {
		for (file = files; file != NULL; file = file->next) {
			init_result = initial_one_var_file_in_mount_path_locked(
				account, file->mount_path);
			if (init_result != 0) {
				first_error = errno ? errno : EIO;
				break;
			}
			file->created = 1;
		}
	}

	/* Publish account identity only after every volume is initialized. */
	if (first_error == 0 &&
	    disk_share_account_inventory_add_locked(
		&disk_share_account_nvram_operations, &transaction,
		acc_num, MAX_ACCOUNT_NUM, account_list, account, password) != 0)
		first_error = errno ? errno : EIO;

	/* Failed account publication must remove only metadata created by this call. */
	if (first_error != 0 && files) {
		rollback_error = 0;
		if (disk_share_account_created_files_rollback(
			    files, unlink, &rollback_error) != 0) {
			logmessage("disk_share",
				   "new account metadata rollback failed (errno %d)",
				   rollback_error);
			first_error = EIO;
		}
	}

	while (files) {
		next_file = files->next;
		free(files);
		files = next_file;
	}
	free_2_dimension_list(&acc_num, &account_list);
	free_disk_data(disk_list);
	if (transaction_started) {
		errno = 0;
		if (disk_share_account_nvram_end(&transaction) != 0) {
			int transaction_error = errno ? errno : EIO;

			if (first_error == 0)
				first_error = transaction_error;
			else
				logmessage("disk_share",
					   "account NVRAM transaction release failed (errno %d)",
					   transaction_error);
		}
	}

	/* Reload Samba after the filesystem and NVRAM changes have settled. */
	errno = 0;
	service_result = system("/sbin/run_samba");
	service_error = disk_share_command_status(service_result, errno);
	if (service_error != 0 && first_error == 0)
		first_error = service_error;
	if (first_error != 0) {
		errno = first_error;
		return -1;
	}
	return 0;
}

int del_account(const char *const account)
{
	disk_info_t *disk_list = NULL, *follow_disk;
	partition_info_t *follow_partition;
	int acc_num = 0, account_count, target = -1;
	struct nvram_transaction transaction;
	char **account_list = NULL;
	int result, i, first_error = 0, saved_errno;
	int service_result, service_error;
	char var_file[256];

	if (!is_valid_account_name(account) || !strcmp(account, FTP_ANONYMOUS_USER))
		return -1;

	/* Preserve a coherent source inventory; revalidate it under the writer lease. */
	if (get_account_list(&acc_num, &account_list) != 0)
		return -1;
	if (acc_num <= 0) {
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}
	account_count = acc_num;

	for (i = 0; i < account_count; ++i) {
		if (!strcmp(account_list[i], account)) {
			target = i;
			break;
		}
	}
	if (target < 0) {
		free_2_dimension_list(&account_count, &account_list);
		return -1;
	}

	/*
	 * Resolve a complete volume snapshot before changing NVRAM. A failed
	 * inventory must not delete the account while leaving unknown metadata.
	 */
	if (read_disk_data_checked(&disk_list) != 0) {
		saved_errno = errno ? errno : EIO;
		free_2_dimension_list(&account_count, &account_list);
		errno = saved_errno;
		return -1;
	}

	/*
	 * Retain the exclusive account writer lease across NVRAM compaction and
	 * every volume unlink. Otherwise a new account with the same name could
	 * publish new metadata between deletion and stale-file cleanup.
	 */
	if (disk_share_account_nvram_begin(&transaction) != 0) {
		saved_errno = errno ? errno : EIO;
		free_2_dimension_list(&account_count, &account_list);
		free_disk_data(disk_list);
		errno = saved_errno;
		return -1;
	}

	result = disk_share_account_inventory_remove_locked(
		&disk_share_account_nvram_operations, &transaction,
		account_count, target, MAX_ACCOUNT_NUM, account_list);
	if (result != 0) {
		saved_errno = errno ? errno : EIO;
		free_2_dimension_list(&account_count, &account_list);
		free_disk_data(disk_list);
		errno = 0;
		if (disk_share_account_nvram_end(&transaction) != 0)
			logmessage("disk_share", "account NVRAM transaction release failed after delete error (errno %d)",
				   errno ? errno : EIO);
		errno = saved_errno;
		return -1;
	}

	free_2_dimension_list(&account_count, &account_list);

	/* Block same-name account creation until all old volume metadata is gone. */
	for (follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions;
		     follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point == NULL)
				continue;

			if (disk_share_format_account_var_path(
				    var_file, sizeof(var_file),
				    follow_partition->mount_point, account) != 0) {
				if (first_error == 0)
					first_error = errno ? errno : EIO;
				continue;
			}

			if (unlink(var_file) != 0 && errno != ENOENT &&
			    first_error == 0)
				first_error = errno ? errno : EIO;
		}
	}
	free_disk_data(disk_list);
	errno = 0;
	if (disk_share_account_nvram_end(&transaction) != 0 && first_error == 0)
		first_error = errno ? errno : EIO;

	/* Reload services even when stale metadata could not be removed. */
	errno = 0;
	service_result = system("/sbin/run_ftpsamba");
	service_error = disk_share_command_status(service_result, errno);
	if (service_error != 0 && first_error == 0)
		first_error = service_error;
	if (first_error != 0) {
		saved_errno = first_error;
		errno = saved_errno;
		return -1;
	}

	return 0;
}
// "new_account" can be the same with "account"!
int mod_account(const char *const account, const char *const new_account, const char *const new_password)
{
	disk_info_t *disk_list = NULL, *follow_disk;
	partition_info_t *follow_partition;
	int i, acc_num = 0, account_order = -1;
	char **account_list = NULL;
	int rename_required, password_update;
	char var_file[256], new_var_file[256];
	struct stat old_stat, new_stat;
	struct disk_share_account_rename_entry *moves = NULL, *moves_tail = NULL;
	struct disk_share_account_rename_entry *move, *next_move;
	int first_error = 0, saved_errno, rollback_error = 0;
	int service_result, service_error;
	int transaction_started = 0, moves_applied = 0;
	struct nvram_transaction transaction;

	if (!is_valid_account_name(account) || !strcmp(account, FTP_ANONYMOUS_USER))
		return -1;
	if (new_account && *new_account &&
	    (!is_valid_account_name(new_account) || is_reserved_account_name(new_account)))
		return -1;
	if (new_password && *new_password &&
	    !is_valid_account_password(new_password))
		return -1;
	if ((new_account == NULL || strlen(new_account) <= 0) &&
	    (new_password == NULL || strlen(new_password) <= 0))
		return -1;

	/* Keep the exact identity snapshot until the writer lease is acquired. */
	if (get_account_list(&acc_num, &account_list) != 0)
		return -1;
	for (i = 0; i < acc_num; ++i) {
		if (new_account && *new_account && strcmp(new_account, account) &&
		    !strcmp(account_list[i], new_account)) {
			free_2_dimension_list(&acc_num, &account_list);
			return -1;
		}
		if (!strcmp(account, account_list[i]))
			account_order = i;
	}
	if (account_order == -1) {
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}
	if (new_account && *new_account && !strcmp(new_account, FTP_ANONYMOUS_USER)) {
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}

	rename_required = new_account && *new_account &&
		strcmp(new_account, account) != 0;
	password_update = new_password && *new_password;

	/*
	 * Preflight the complete rename plan without changing files. The exact
	 * NVRAM identity snapshot is revalidated under its writer lease below.
	 */
	if (rename_required) {
		if (read_disk_data_checked(&disk_list) != 0) {
			first_error = errno ? errno : EIO;
		} else {
			for (follow_disk = disk_list;
			     follow_disk != NULL && first_error == 0;
			     follow_disk = follow_disk->next) {
				for (follow_partition = follow_disk->partitions;
				     follow_partition != NULL && first_error == 0;
				     follow_partition = follow_partition->next) {
					if (follow_partition->mount_point == NULL)
						continue;

					if (disk_share_format_account_var_path(
						    var_file, sizeof(var_file),
						    follow_partition->mount_point, account) != 0) {
						first_error = errno ? errno : EIO;
						break;
					}
					if (disk_share_format_account_var_path(
						    new_var_file, sizeof(new_var_file),
						    follow_partition->mount_point, new_account) != 0) {
						first_error = errno ? errno : EIO;
						break;
					}

					/* Missing source metadata is idempotent. */
					if (lstat(var_file, &old_stat) != 0) {
						saved_errno = errno ? errno : EIO;
						if (saved_errno == ENOENT)
							continue;
						first_error = saved_errno;
						break;
					}
					if (!S_ISREG(old_stat.st_mode)) {
						first_error = EINVAL;
						break;
					}
					if (lstat(new_var_file, &new_stat) == 0) {
						first_error = EEXIST;
						break;
					}
					saved_errno = errno ? errno : EIO;
					if (saved_errno != ENOENT) {
						first_error = saved_errno;
						break;
					}

					move = (struct disk_share_account_rename_entry *)
						calloc(1, sizeof(*move));
					if (!move) {
						first_error = errno ? errno : ENOMEM;
						break;
					}
					strcpy(move->source, var_file);
					strcpy(move->destination, new_var_file);
					move->previous = moves_tail;
					if (moves_tail)
						moves_tail->next = move;
					else
						moves = move;
					moves_tail = move;
				}
			}
		}
	}

	/*
	 * Serialize add/delete/rename/password operations. Revalidate the old
	 * account and destination name before touching per-volume metadata.
	 */
	if (first_error == 0) {
		if (disk_share_account_nvram_begin(&transaction) != 0) {
			first_error = errno ? errno : EIO;
		} else {
			transaction_started = 1;
			if (disk_share_account_inventory_update_validate_locked(
				    &disk_share_account_nvram_operations, &transaction,
				    acc_num, MAX_ACCOUNT_NUM, account_list, account_order,
				    account, rename_required ? new_account : NULL) != 0)
				first_error = errno ? errno : EIO;
		}
	}

	if (first_error == 0 && rename_required && moves) {
		rollback_error = 0;
		if (disk_share_account_rename_entries_apply(
			    moves, rename, &rollback_error) != 0) {
			saved_errno = errno ? errno : EIO;
			if (rollback_error != 0) {
				logmessage("disk_share",
					   "account metadata rename rollback failed (errno %d)",
					   rollback_error);
				first_error = EIO;
			} else {
				first_error = saved_errno;
			}
		} else {
			moves_applied = 1;
		}
	}

	if (first_error == 0 &&
	    disk_share_account_inventory_update_locked(
		&disk_share_account_nvram_operations, &transaction,
		acc_num, MAX_ACCOUNT_NUM, account_list, account_order,
		account, rename_required ? new_account : NULL,
		password_update ? new_password : NULL) != 0)
		first_error = errno ? errno : EIO;

	/* A failed NVRAM publish must compensate every already-renamed file. */
	if (first_error != 0 && moves_applied && moves) {
		rollback_error = 0;
		if (disk_share_account_rename_entries_rollback(
			    moves, rename, &rollback_error) != 0) {
			logmessage("disk_share",
				   "account metadata rollback after NVRAM failure failed (errno %d)",
				   rollback_error);
			first_error = EIO;
		}
	}

	while (moves) {
		next_move = moves->next;
		free(moves);
		moves = next_move;
	}
	free_2_dimension_list(&acc_num, &account_list);
	free_disk_data(disk_list);
	if (transaction_started) {
		errno = 0;
		if (disk_share_account_nvram_end(&transaction) != 0) {
			int transaction_error = errno ? errno : EIO;

			if (first_error == 0)
				first_error = transaction_error;
			else
				logmessage("disk_share",
					   "account NVRAM transaction release failed (errno %d)",
					   transaction_error);
		}
	}

	/* Reload services only after both stores and the writer lease settle. */
	errno = 0;
	service_result = system("/sbin/run_ftpsamba");
	service_error = disk_share_command_status(service_result, errno);
	if (service_error != 0 && first_error == 0)
		first_error = service_error;
	if (first_error != 0) {
		errno = first_error;
		return -1;
	}
	return 0;
}

static int
disk_share_update_folder_permission_file(const char *mount_path,
                                         const char *account,
                                         const char *target,
                                         const char *replacement);

int add_folder(const char *const mount_path, const char *const folder)
{
	int result, saved_errno, transaction_started = 0, directory_created = 0;
	int permission_init_started = 0, rollback_error = 0;
	int acc_num = 0, i;
	char **account_list = NULL;
	struct nvram_transaction transaction;
	size_t mount_len, folder_len, full_len, target_len;
	char *full_path = NULL, *target = NULL;
	mode_t old_umask;

	if (!mount_path || !*mount_path || !is_valid_folder_name(folder)) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * Capture the account list before mutation, then revalidate it after
	 * acquiring the same writer lease used by account add/delete/rename.
	 */
	if (get_account_list(&acc_num, &account_list) != 0)
		return -1;

	mount_len = strlen(mount_path);
	folder_len = strlen(folder);
	if (mount_len > (size_t)-1 - 2 ||
	    folder_len > (size_t)-1 - mount_len - 2) {
		errno = ENAMETOOLONG;
		goto failed;
	}
	full_len = mount_len + 1 + folder_len;
	full_path = (char *)malloc(full_len + 1);
	if (!full_path)
		goto failed;
	memcpy(full_path, mount_path, mount_len);
	full_path[mount_len] = '/';
	memcpy(full_path + mount_len + 1, folder, folder_len + 1);

	target_len = folder_len + 2;
	if (target_len == (size_t)-1) {
		errno = ENAMETOOLONG;
		goto failed;
	}
	target = (char *)malloc(target_len + 1);
	if (!target)
		goto failed;
	target[0] = '*';
	memcpy(target + 1, folder, folder_len);
	target[target_len - 1] = '=';
	target[target_len] = '\0';

	if (disk_share_account_nvram_begin(&transaction) != 0)
		goto failed;
	transaction_started = 1;
	if (disk_share_account_inventory_matches_locked(
		    &disk_share_account_nvram_operations, &transaction,
		    acc_num, MAX_ACCOUNT_NUM, account_list) != 0)
		goto failed;

	/* A failed folder-list read is an error, not evidence of a free name. */
	result = test_if_exist_folder_in_mount_path(mount_path, folder);
	if (result != 0) {
		if (result > 0)
			errno = EEXIST;
		goto failed;
	}

	old_umask = umask(0000);
	result = mkdir(full_path, 0777);
	(void)umask(old_umask);
	if (result != 0)
		goto failed;
	directory_created = 1;

	/* Publish the live directory before initializing its permission records. */
	if (initial_folder_list_in_mount_path(mount_path) != 0)
		goto failed;

	/* Reuse the already-validated inventory; do not nest an NVRAM read lock. */
	permission_init_started = 1;
	if (create_if_no_var_files_with_accounts(
		    mount_path, acc_num, account_list) != 0)
		goto failed;

	free(target);
	free(full_path);
	free_2_dimension_list(&acc_num, &account_list);
	errno = 0;
	if (disk_share_account_nvram_end(&transaction) != 0)
		return -1;
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (directory_created) {
		int remove_result, remove_errno;

		remove_result = rmdir(full_path);
		remove_errno = errno;
		if (!disk_share_folder_rollback_can_remove_permissions(
			    remove_result, remove_errno)) {
			if (rollback_error == 0)
				rollback_error = remove_errno ? remove_errno : EIO;
			logmessage("disk_share",
				   "cannot roll back new share directory %s (errno %d)",
				   full_path, remove_errno ? remove_errno : EIO);
		} else if (permission_init_started) {
			/*
			 * Some account files may already contain the new permission
			 * row. Remove it from every known file when rolling the folder
			 * back so a later re-create cannot inherit partial state.
			 */
			for (i = 0; i < acc_num; ++i) {
				if (disk_share_update_folder_permission_file(
					    mount_path, account_list[i], target, NULL) != 0 &&
				    rollback_error == 0)
					rollback_error = errno ? errno : EIO;
			}
			if (disk_share_update_folder_permission_file(
				    mount_path, FTP_ANONYMOUS_USER, target, NULL) != 0 &&
			    rollback_error == 0)
				rollback_error = errno ? errno : EIO;
		}

		/* Rebuild from disk whether rmdir succeeded or had to be abandoned. */
		if (initial_folder_list_in_mount_path(mount_path) != 0 &&
		    rollback_error == 0)
			rollback_error = errno ? errno : EIO;
	}
	if (rollback_error != 0) {
		logmessage("disk_share",
			   "share folder creation rollback incomplete (errno %d)",
			   rollback_error);
		saved_errno = EIO;
	}
	free(target);
	free(full_path);
	free_2_dimension_list(&acc_num, &account_list);
	if (transaction_started) {
		errno = 0;
		if (disk_share_account_nvram_end(&transaction) != 0)
			logmessage("disk_share",
				   "folder creation transaction release failed (errno %d)",
				   errno ? errno : EIO);
	}
	errno = saved_errno;
	return -1;
}

static int
disk_share_update_folder_permission_file(const char *mount_path,
                                         const char *account,
                                         const char *target,
                                         const char *replacement)
{
	char var_file[256];
	char *var_info = NULL, *updated_info = NULL;
	size_t updated_len = 0;
	int saved_errno;

	if (disk_share_format_account_var_path(var_file, sizeof(var_file),
	                                       mount_path, account) != 0)
		return -1;

	var_info = read_regular_file_nofollow(var_file);
	if (!var_info) {
		saved_errno = errno ? errno : EIO;
		if (saved_errno == ENOENT) {
			errno = 0;
			return 0;
		}
		errno = saved_errno;
		return -1;
	}

	if (disk_share_permission_rewrite(var_info, target, replacement,
	                                  &updated_info, &updated_len) != 0) {
		saved_errno = errno ? errno : EIO;
		free(var_info);
		if (saved_errno == ENOENT) {
			errno = 0;
			return 0;
		}
		errno = saved_errno;
		return -1;
	}

	if (disk_share_replace_file_atomic(var_file, updated_info,
	                                   updated_len) != 0) {
		saved_errno = errno ? errno : EIO;
		free(var_info);
		free(updated_info);
		errno = saved_errno;
		return -1;
	}

	free(var_info);
	free(updated_info);
	return 0;
}


static void
disk_share_permission_batch_free(
	struct disk_share_permission_batch_entry *entries, int count)
{
	int i;

	if (!entries)
		return;
	for (i = 0; i < count; ++i) {
		free(entries[i].before);
		free(entries[i].after);
	}
	free(entries);
}

static int
disk_share_permission_batch_prepare_rewrite(
	const char *mount_path, int acc_num, char **account_list,
	const char *target, const char *replacement,
	struct disk_share_permission_batch_entry **entries_out,
	int *entry_count_out)
{
	struct disk_share_permission_batch_entry *entries = NULL;
	int i, count, saved_errno;
	const char *account;

	if (entries_out)
		*entries_out = NULL;
	if (entry_count_out)
		*entry_count_out = 0;
	if (!mount_path || !*mount_path || acc_num < 0 ||
	    acc_num > MAX_ACCOUNT_NUM ||
	    (acc_num > 0 && !account_list) || !target ||
	    !entries_out || !entry_count_out) {
		errno = EINVAL;
		return -1;
	}

	count = acc_num + 1;
	entries = (struct disk_share_permission_batch_entry *)
		calloc((size_t)count, sizeof(*entries));
	if (!entries)
		return -1;

	for (i = 0; i < count; ++i) {
		account = i < acc_num ? account_list[i] : FTP_ANONYMOUS_USER;
		if (disk_share_format_account_var_path(
			    entries[i].path, sizeof(entries[i].path),
			    mount_path, account) != 0)
			goto failed;

		entries[i].before = read_regular_file_nofollow(entries[i].path);
		if (!entries[i].before) {
			saved_errno = errno ? errno : EIO;
			if (saved_errno == ENOENT) {
				errno = 0;
				continue;
			}
			errno = saved_errno;
			goto failed;
		}
		entries[i].before_len = strlen(entries[i].before);
		if (disk_share_permission_rewrite(
			    entries[i].before, target, replacement,
			    &entries[i].after, &entries[i].after_len) != 0) {
			saved_errno = errno ? errno : EIO;
			if (saved_errno == ENOENT) {
				errno = 0;
				continue;
			}
			errno = saved_errno;
			goto failed;
		}
		entries[i].needs_update = 1;
	}

	*entries_out = entries;
	*entry_count_out = count;
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	disk_share_permission_batch_free(entries, count);
	errno = saved_errno;
	return -1;
}

static int
disk_share_mkdir_staging_directory(const char *path)
{
	return mkdir(path, 0700);
}

static int
disk_share_create_folder_staging_paths(
	const char *mount_path, const char *folder,
	char *staging_dir, size_t staging_dir_size,
	char *staging_path, size_t staging_path_size)
{
	int path_len, saved_errno;

	if (!folder || !*folder || !staging_path || staging_path_size == 0) {
		errno = EINVAL;
		return -1;
	}
	if (disk_share_reserve_staging_directory(
		    mount_path, staging_dir, staging_dir_size, (long)getpid(),
		    disk_share_mkdir_staging_directory) != 0)
		return -1;

	path_len = snprintf(staging_path, staging_path_size,
	                    "%s/%s", staging_dir, folder);
	if (path_len < 0 || (size_t)path_len >= staging_path_size) {
		saved_errno = ENAMETOOLONG;
		(void)rmdir(staging_dir);
		errno = saved_errno;
		return -1;
	}
	return 0;
}

static int
disk_share_rename_folder_path(const char *old_path, const char *new_path)
{
	return rename(old_path, new_path);
}

static int
disk_share_refresh_folder_snapshot(const char *mount_path)
{
	return initial_folder_list_in_mount_path(mount_path);
}

static int
disk_share_remove_folder_path(const char *path)
{
	return rmdir(path);
}


int del_folder(const char *const mount_path, const char *const folder)
{
	int result, saved_errno, rollback_error = 0;
	int acc_num = 0, transaction_started = 0, staging_applied = 0;
	int permission_batch_count = 0, staging_dir_created = 0;
	struct nvram_transaction transaction;
	struct stat folder_status;
	char **account_list = NULL;
	char var_file[256], staging_dir[256], staging_path[512];
	char *target = NULL, *full_path = NULL;
	struct disk_share_permission_batch_entry *permission_batch = NULL;
	size_t mount_len, folder_len, path_len, target_len;

	if (!mount_path || !*mount_path || !is_valid_folder_name(folder)) {
		errno = EINVAL;
		return -1;
	}

	mount_len = strlen(mount_path);
	folder_len = strlen(folder);
	if (folder_len > (size_t)-1 - 2 ||
	    mount_len > (size_t)-1 - folder_len - 2) {
		errno = ENAMETOOLONG;
		return -1;
	}
	path_len = mount_len + folder_len + 1;
	full_path = (char *)malloc(path_len + 1);
	if (!full_path)
		return -1;
	memcpy(full_path, mount_path, mount_len);
	full_path[mount_len] = '/';
	memcpy(full_path + mount_len + 1, folder, folder_len + 1);

	target_len = folder_len + 2;
	target = (char *)malloc(target_len + 1);
	if (!target)
		goto failed;
	target[0] = '*';
	memcpy(target + 1, folder, folder_len);
	target[target_len - 1] = '=';
	target[target_len] = '\0';

	result = get_account_list(&acc_num, &account_list);
	if (result != 0)
		goto failed;
	if (acc_num < 0 || acc_num > MAX_ACCOUNT_NUM) {
		errno = EINVAL;
		goto failed;
	}
	for (result = 0; result < acc_num; ++result) {
		if (disk_share_format_account_var_path(
			    var_file, sizeof(var_file), mount_path,
			    account_list[result]) != 0)
			goto failed;
	}
	if (disk_share_format_account_var_path(
		    var_file, sizeof(var_file), mount_path,
		    FTP_ANONYMOUS_USER) != 0)
		goto failed;

	/*
	 * Serialize account identity changes while preparing, applying and
	 * potentially compensating the folder's per-account permission batch.
	 */
	if (disk_share_account_nvram_begin(&transaction) != 0)
		goto failed;
	transaction_started = 1;
	if (disk_share_account_inventory_matches_locked(
		    &disk_share_account_nvram_operations, &transaction,
		    acc_num, MAX_ACCOUNT_NUM, account_list) != 0)
		goto failed;

	result = test_if_exist_folder_in_mount_path(mount_path, folder);
	if (result < 0)
		goto failed;
	if (lstat(full_path, &folder_status) != 0)
		goto failed;
	if (!S_ISDIR(folder_status.st_mode)) {
		errno = ENOTDIR;
		goto failed;
	}

	/* Read/transform every metadata file before making any change. */
	if (disk_share_permission_batch_prepare_rewrite(
		    mount_path, acc_num, account_list, target, NULL,
		    &permission_batch, &permission_batch_count) != 0)
		goto failed;
	if (disk_share_create_folder_staging_paths(
		    mount_path, folder, staging_dir, sizeof(staging_dir),
		    staging_path, sizeof(staging_path)) != 0)
		goto failed;
	staging_dir_created = 1;

	/*
	 * The policy helper stages the directory, refreshes the visible list,
	 * publishes the complete ACL batch, and compensates all three on failure.
	 * The output flag keeps the staging parent when a failed rename-back leaves
	 * the directory there, so data is never discarded during cleanup.
	 */
	if (disk_share_folder_delete_batch(
		    full_path, staging_path, mount_path,
		    permission_batch, permission_batch_count,
		    disk_share_replace_file_atomic, disk_share_rename_folder_path,
		    disk_share_remove_folder_path, disk_share_refresh_folder_snapshot,
		    &staging_applied, &rollback_error) != 0) {
		if (rollback_error != 0)
			logmessage("disk_share",
				   "folder deletion compensation incomplete (errno %d)",
				   rollback_error);
		goto failed;
	}
	if (rmdir(staging_dir) == 0 || errno == ENOENT)
		staging_dir_created = 0;
	else
		logmessage("disk_share",
			   "empty folder deletion staging directory cleanup failed (errno %d)",
			   errno ? errno : EIO);

	disk_share_permission_batch_free(permission_batch, permission_batch_count);
	free(target);
	free(full_path);
	free_2_dimension_list(&acc_num, &account_list);
	errno = 0;
	if (disk_share_account_nvram_end(&transaction) != 0)
		return -1;
	transaction_started = 0;
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (staging_dir_created && !staging_applied) {
		if (rmdir(staging_dir) != 0 && errno != ENOENT)
			logmessage("disk_share",
				   "folder deletion staging directory cleanup failed (errno %d)",
				   errno ? errno : EIO);
	}
	disk_share_permission_batch_free(permission_batch, permission_batch_count);
	free(target);
	free(full_path);
	free_2_dimension_list(&acc_num, &account_list);
	if (transaction_started) {
		errno = 0;
		if (disk_share_account_nvram_end(&transaction) != 0)
			logmessage("disk_share",
				   "folder deletion transaction release failed (errno %d)",
				   errno ? errno : EIO);
	}
	errno = saved_errno;
	return -1;
}


int mod_folder(const char *const mount_path, const char *const folder, const char *const new_folder)
{
	int result, saved_errno, rollback_error = 0;
	int acc_num = 0, transaction_started = 0;
	int permission_batch_count = 0;
	char **account_list = NULL;
	struct nvram_transaction transaction;
	char var_file[256];
	char *target = NULL, *new_target = NULL;
	char *full_path = NULL, *new_full_path = NULL;
	struct stat target_stat;
	struct disk_share_permission_batch_entry *permission_batch = NULL;
	size_t mount_len, folder_len, new_folder_len, path_len;
	size_t target_len, new_target_len;

	if (!mount_path || !*mount_path || !is_valid_folder_name(folder) ||
	    !is_valid_folder_name(new_folder)) {
		errno = EINVAL;
		return -1;
	}

	mount_len = strlen(mount_path);
	folder_len = strlen(folder);
	new_folder_len = strlen(new_folder);
	if (folder_len > (size_t)-1 - 2 ||
	    new_folder_len > (size_t)-1 - 2 ||
	    mount_len > (size_t)-1 - folder_len - 2 ||
	    mount_len > (size_t)-1 - new_folder_len - 2) {
		errno = ENAMETOOLONG;
		return -1;
	}
	path_len = mount_len + folder_len + 1;
	full_path = (char *)malloc(path_len + 1);
	if (!full_path)
		goto failed;
	memcpy(full_path, mount_path, mount_len);
	full_path[mount_len] = '/';
	memcpy(full_path + mount_len + 1, folder, folder_len + 1);

	path_len = mount_len + new_folder_len + 1;
	new_full_path = (char *)malloc(path_len + 1);
	if (!new_full_path)
		goto failed;
	memcpy(new_full_path, mount_path, mount_len);
	new_full_path[mount_len] = '/';
	memcpy(new_full_path + mount_len + 1, new_folder, new_folder_len + 1);

	result = get_account_list(&acc_num, &account_list);
	if (result != 0)
		goto failed;
	if (acc_num < 0 || acc_num > MAX_ACCOUNT_NUM) {
		errno = EINVAL;
		goto failed;
	}
	for (result = 0; result < acc_num; ++result) {
		if (disk_share_format_account_var_path(
			    var_file, sizeof(var_file), mount_path,
			    account_list[result]) != 0)
			goto failed;
	}
	if (disk_share_format_account_var_path(
		    var_file, sizeof(var_file), mount_path,
		    FTP_ANONYMOUS_USER) != 0)
		goto failed;

	if (disk_share_account_nvram_begin(&transaction) != 0)
		goto failed;
	transaction_started = 1;
	if (disk_share_account_inventory_matches_locked(
		    &disk_share_account_nvram_operations, &transaction,
		    acc_num, MAX_ACCOUNT_NUM, account_list) != 0)
		goto failed;

	errno = 0;
	result = lstat(full_path, &target_stat);
	if (disk_share_folder_entry_status(
		    result, errno,
		    result == 0 ? target_stat.st_mode : (mode_t)0) != 1) {
		if (result != 0 && errno == 0)
			errno = EIO;
		else if (result == 0)
			errno = ENOTDIR;
		goto failed;
	}
	result = test_if_exist_folder_in_mount_path(mount_path, folder);
	if (result < 0)
		goto failed;
	if (result == 0 && initial_folder_list_in_mount_path(mount_path) != 0)
		goto failed;

	if (strcmp(folder, new_folder) == 0)
		goto unchanged;

	result = test_if_exist_folder_in_mount_path(mount_path, new_folder);
	if (result < 0)
		goto failed;
	if (result != 0) {
		errno = EEXIST;
		goto failed;
	}
	errno = 0;
	if (lstat(new_full_path, &target_stat) == 0) {
		errno = EEXIST;
		goto failed;
	}
	if (errno != ENOENT)
		goto failed;

	target_len = folder_len + 2;
	new_target_len = new_folder_len + 2;
	target = (char *)malloc(target_len + 1);
	new_target = (char *)malloc(new_target_len + 1);
	if (!target || !new_target)
		goto failed;
	target[0] = '*';
	memcpy(target + 1, folder, folder_len);
	target[target_len - 1] = '=';
	target[target_len] = '\0';
	new_target[0] = '*';
	memcpy(new_target + 1, new_folder, new_folder_len);
	new_target[new_target_len - 1] = '=';
	new_target[new_target_len] = '\0';

	if (disk_share_permission_batch_prepare_rewrite(
		    mount_path, acc_num, account_list, target, new_target,
		    &permission_batch, &permission_batch_count) != 0)
		goto failed;

	if (disk_share_folder_rename_batch(
		    full_path, new_full_path, mount_path,
		    permission_batch, permission_batch_count,
		    disk_share_replace_file_atomic,
		    disk_share_rename_folder_path,
		    disk_share_refresh_folder_snapshot, &rollback_error) != 0) {
		if (rollback_error != 0)
			logmessage("disk_share",
				   "folder rename compensation incomplete (errno %d)",
				   rollback_error);
		goto failed;
	}

	disk_share_permission_batch_free(permission_batch, permission_batch_count);
	free(target);
	free(new_target);
	free(full_path);
	free(new_full_path);
	free_2_dimension_list(&acc_num, &account_list);
	errno = 0;
	if (disk_share_account_nvram_end(&transaction) != 0)
		return -1;
	transaction_started = 0;
	return 0;

unchanged:
	disk_share_permission_batch_free(permission_batch, permission_batch_count);
	free(target);
	free(new_target);
	free(full_path);
	free(new_full_path);
	free_2_dimension_list(&acc_num, &account_list);
	errno = 0;
	if (disk_share_account_nvram_end(&transaction) != 0)
		return -1;
	transaction_started = 0;
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	disk_share_permission_batch_free(permission_batch, permission_batch_count);
	free(target);
	free(new_target);
	free(full_path);
	free(new_full_path);
	free_2_dimension_list(&acc_num, &account_list);
	if (transaction_started) {
		errno = 0;
		if (disk_share_account_nvram_end(&transaction) != 0)
			logmessage("disk_share",
				   "folder rename transaction release failed (errno %d)",
				   errno ? errno : EIO);
	}
	errno = saved_errno;
	return -1;
}


int test_if_exist_account(const char *const account)
{
	int acc_num = 0;
	char **account_list = NULL;
	int result, i;

	if (!is_valid_account_name(account))
		return -1;
	if (!strcmp(account, FTP_ANONYMOUS_USER))
		return 1;

	if (get_account_list(&acc_num, &account_list) != 0)
		return -1;

	result = 0;
	for (i = 0; i < acc_num; ++i)
		if (!strcmp(account, account_list[i])) {
			result = 1;
			break;
		}

	free_2_dimension_list(&acc_num, &account_list);
	return result;
}

int test_if_exist_folder_in_mount_path(const char *const mount_path, const char *const folder)
{
	int sh_num = 0;
	char **folder_list = NULL;
	int result, i;

	if (!mount_path || !*mount_path || !is_valid_folder_name(folder))
		return -1;

	result = get_folder_list_in_mount_path(mount_path, &sh_num, &folder_list);
	if (result != 0) {
		free_2_dimension_list(&sh_num, &folder_list);
		return -1;
	}

	result = 0;
	for (i = 0; i < sh_num; ++i)
		if (!upper_strcmp(folder, folder_list[i])) {
			result = 1;
			break;
		}

	free_2_dimension_list(&sh_num, &folder_list);
	return result;
}

// for FTP: root dir is POOL_MOUNT_ROOT(/media)
int how_many_layer(int is_chroot, const char *basedir, char **mount_path, char **share)
{
	char *follow_info, *follow_info_end;
	const char *component;
	size_t root_len, path_len, prefix_len, share_len, component_len;
	int layer = 0;

	if (!basedir || !mount_path || !share || basedir[0] != '/')
		return -1;
	*mount_path = NULL;
	*share = NULL;

	root_len = strlen(POOL_MOUNT_ROOT);
	path_len = strlen(basedir);
	if (!is_chroot &&
	    (path_len < root_len ||
	     strncmp(basedir, POOL_MOUNT_ROOT, root_len) != 0 ||
	     (basedir[root_len] != '\0' && basedir[root_len] != '/')))
		return -1;

	/* Reject non-canonical path components before deriving mount/share names. */
	if (path_len > 1 && basedir[path_len - 1] == '/')
		return -1;
	component = basedir + 1;
	while (*component) {
		const char *component_start = component;
		while (*component && *component != '/')
			++component;
		component_len = (size_t)(component - component_start);
		if (component_len == 0 ||
		    (component_len == 1 && component_start[0] == '.') ||
		    (component_len == 2 && component_start[0] == '.' &&
		     component_start[1] == '.'))
			return -1;
		if (*component == '/') {
			++component;
			if (*component == '/')
				return -1;
		}
	}

	if (is_chroot)
		layer = BASE_LAYER;
	if (!strcmp(basedir, "/"))
		return layer;

	if (path_len > 1)
		layer++;

	follow_info = (char *)basedir;
	while (*follow_info != '\0' &&
	       (follow_info = index(follow_info + 1, '/')) != NULL)
		++layer;

	if (layer >= MOUNT_LAYER) {
		follow_info = (char *)basedir;
		if (!is_chroot)
			follow_info += root_len;
		/* For unchrooted paths, follow_info points at the slash after /media.
		 * For chrooted paths it points at the leading slash. */
		follow_info_end = index(follow_info + 1, '/');
		if (follow_info_end)
			prefix_len = (size_t)(follow_info_end - basedir);
		else
			prefix_len = path_len;
		if (layer >= SHARE_LAYER) {
			if (!follow_info_end)
				return -1;
			follow_info = follow_info_end;
		}

		*mount_path = (char *)malloc(prefix_len + 1);
		if (!*mount_path)
			return -1;
		memcpy(*mount_path, basedir, prefix_len);
		(*mount_path)[prefix_len] = '\0';
	}

	if (layer >= SHARE_LAYER) {
		if (!follow_info || *follow_info != '/') {
			free(*mount_path);
			*mount_path = NULL;
			return -1;
		}
		++follow_info;
		follow_info_end = index(follow_info, '/');
		share_len = follow_info_end ?
			(size_t)(follow_info_end - follow_info) : strlen(follow_info);
		if (share_len == 0) {
			free(*mount_path);
			*mount_path = NULL;
			return -1;
		}
		*share = (char *)malloc(share_len + 1);
		if (!*share) {
			free(*mount_path);
			*mount_path = NULL;
			return -1;
		}
		memcpy(*share, follow_info, share_len);
		(*share)[share_len] = '\0';
	}

	return layer;
}


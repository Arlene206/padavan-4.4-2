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
	disk_share_account_name_reserved
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
	int i, count;
	long parsed_count;
	size_t list_file_len, list_info_len, name_len;
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

	if (!test_if_file(list_file))
		initial_folder_list_in_mount_path(mount_path);

	list_info = read_whole_file(list_file);
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
	}

	free(list_info);
	*sh_num = count;
	*folder_list = tmp_folder_list;
	return 0;

invalid_entries:
	free_2_dimension_list(&count, &tmp_folder_list);
invalid_list:
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

		if (dp->d_name[0] == '.')
			continue;
		if (test_if_System_folder(dp->d_name) == 1)
			continue;

		name_len = strlen(dp->d_name);
		if (!is_valid_folder_name(dp->d_name))
			continue;
		if (mount_len > (size_t)-1 - name_len - 2)
			goto failed;

		path_len = mount_len + name_len + 1;
		testdir = (char *)malloc(path_len + 1);
		if (!testdir)
			goto failed;

		snprintf(testdir, path_len + 1, "%s/%s", mount_path, dp->d_name);
		if (!test_if_dir(testdir)) {
			free(testdir);
			continue;
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

int initial_one_var_file_in_mount_path(const char *const account,
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
clear_share_var_files(const char *mount_path)
{
	DIR *dir;
	struct dirent *entry;
	char account[32 + 1], *path;
	int read_status, read_error, managed, saved_errno;
	size_t mount_len, name_len, account_len, path_len;

	if (!mount_path || !*mount_path) {
		errno = EINVAL;
		return -1;
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
		managed = 0;
		if (strcmp(entry->d_name, ".__folder_list.txt") == 0) {
			managed = 1;
		} else if (name_len > 3 + 8 &&
			   strncmp(entry->d_name, ".__", 3) == 0 &&
			   strcmp(entry->d_name + name_len - 8, "_var.txt") == 0) {
			account_len = name_len - 3 - 8;
			if (account_len > 0 && account_len <= 31) {
				memcpy(account, entry->d_name + 3, account_len);
				account[account_len] = '\0';
				managed = is_valid_account_name(account);
			}
		}
		if (!managed)
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

int initial_all_var_file_in_mount_path(const char *const mount_path)
{
	int i;
	int acc_num = 0;
	char **account_list = NULL;

	if (mount_path == NULL || strlen(mount_path) <= 0)
		return -1;

	/* Delete managed files without interpolating the mount path into a shell. */
	if (clear_share_var_files(mount_path) != 0)
		return -1;

	/* Initialize anonymous defaults before account-specific permissions. */
	if (initial_one_var_file_in_mount_path(FTP_ANONYMOUS_USER,
	                                       mount_path) != 0)
		return -1;

	/* Get and initialize every configured account. */
	if (get_account_list(&acc_num, &account_list) != 0) {
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}
	for (i = 0; i < acc_num; ++i) {
		if (initial_one_var_file_in_mount_path(account_list[i],
		                                       mount_path) != 0) {
			int saved_errno = errno ? errno : EIO;

			free_2_dimension_list(&acc_num, &account_list);
			errno = saved_errno;
			return -1;
		}
	}
	free_2_dimension_list(&acc_num, &account_list);

	/* Publish the folder-list snapshot only after all account files exist. */
	if (initial_folder_list_in_mount_path(mount_path) != 0)
		return -1;

	return 0;
}

int create_if_no_var_files(const char *const mount_path)
{
	int acc_num = 0;
	char **account_list = NULL;
	int i, path_len, result;
	char var_file[256];

	if (!mount_path || !*mount_path) {
		errno = EINVAL;
		return -1;
	}

	path_len = snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt",
	                    mount_path, FTP_ANONYMOUS_USER);
	if (path_len < 0 || path_len >= (int)sizeof(var_file)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	if (!test_if_file(var_file))
		result = initial_one_var_file_in_mount_path(FTP_ANONYMOUS_USER,
		                                           mount_path);
	else
		result = modify_if_exist_new_folder(FTP_ANONYMOUS_USER, mount_path);
	if (result != 0)
		return -1;

	if (get_account_list(&acc_num, &account_list) != 0) {
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}

	for (i = 0; i < acc_num; ++i) {
		path_len = snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt",
		                    mount_path, account_list[i]);
		if (path_len < 0 || path_len >= (int)sizeof(var_file)) {
			errno = ENAMETOOLONG;
			result = -1;
			break;
		}
		if (!test_if_file(var_file))
			result = initial_one_var_file_in_mount_path(account_list[i],
			                                           mount_path);
		else
			result = modify_if_exist_new_folder(account_list[i], mount_path);
		if (result != 0)
			break;
	}

	if (result != 0) {
		int saved_errno = errno ? errno : EIO;

		free_2_dimension_list(&acc_num, &account_list);
		errno = saved_errno;
		return -1;
	}
	free_2_dimension_list(&acc_num, &account_list);
	return 0;
}

int modify_if_exist_new_folder(const char *const account,
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

	old_info = read_whole_file(var_file);
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
	var_info = read_whole_file(var_file);
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
	int len, result, saved_errno, samba_right = 0, ftp_right = 0;
	int dms_right = DEFAULT_DMS_RIGHT;
	size_t old_len, target_len, updated_len, separator_len;
	char record_rights[5];

	if (flag < 0 || flag > 3 || !protocol ||
	    (strcmp(protocol, "cifs") && strcmp(protocol, "ftp") &&
	     strcmp(protocol, "dms")) ||
	    !mount_path || !*mount_path || !is_valid_account_name(account) ||
	    !is_valid_folder_name(folder)) {
		errno = EINVAL;
		return -1;
	}

	if (disk_share_format_account_var_path(var_file, sizeof(var_file),
	                                       mount_path, account) != 0)
		return -1;
	var_info = read_whole_file(var_file);
	if (!var_info) {
		saved_errno = errno ? errno : EIO;
		if (saved_errno != ENOENT) {
			errno = saved_errno;
			return -1;
		}
		errno = 0;
		result = initial_one_var_file_in_mount_path(account, mount_path);
		if (result != 0) {
			if (!errno)
				errno = EIO;
			return -1;
		}
		var_info = read_whole_file(var_file);
		if (!var_info) {
			if (!errno)
				errno = EIO;
			return -1;
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
	if (!strcmp(protocol, "cifs"))
		system("/sbin/run_samba");
	return 0;

unchanged:
	free(target);
	free(var_info);
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	free(target);
	free(var_info);
	free(updated_info);
	errno = saved_errno;
	return -1;
}

int add_account(const char *const account, const char *const password)
{
	disk_info_t *disk_list = NULL, *follow_disk;
	partition_info_t *follow_partition;
	int i, acc_num = 0, first_error = 0, init_result;
	int service_result, service_error, rollback_error = 0;
	char **account_list = NULL;
	char nvram_name[32], password_key[32];
	char old_username[128], old_password[128];
	char *old_value = NULL;
	int old_username_present = 0, old_password_present = 0;
	int username_attempted = 0, password_attempted = 0, count_attempted = 0;
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
	free_2_dimension_list(&acc_num, &account_list);

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
				    sizeof(files_tail ? files_tail->mount_path : ((struct disk_share_account_created_file_entry *)0)->mount_path)) {
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

	/* Create metadata on every volume before exposing the account in NVRAM. */
	if (first_error == 0) {
		for (file = files; file != NULL; file = file->next) {
			init_result = initial_one_var_file_in_mount_path(
				account, file->mount_path);
			if (init_result != 0) {
				first_error = errno ? errno : EIO;
				break;
			}
			file->created = 1;
		}
	}

	/*
	 * Preserve any stale tail slot that will be overwritten. Account deletion
	 * compacts the active list but legacy firmware may leave old tail keys.
	 */
	if (first_error == 0) {
		snprintf(nvram_name, sizeof(nvram_name), "acc_username%d", acc_num);
		errno = 0;
		init_result = nvram_get_checked(nvram_name, &old_value);
		if (init_result < 0) {
			first_error = errno ? errno : EIO;
		} else if (init_result == 1 && old_value) {
			if (strlen(old_value) >= sizeof(old_username))
				first_error = ENAMETOOLONG;
			else {
				strcpy(old_username, old_value);
				old_username_present = 1;
			}
		} else if (init_result != 0) {
			first_error = EIO;
		}

		if (first_error == 0) {
			snprintf(password_key, sizeof(password_key), "acc_password%d", acc_num);
			old_value = NULL;
			errno = 0;
			init_result = nvram_get_checked(password_key, &old_value);
			if (init_result < 0) {
				first_error = errno ? errno : EIO;
			} else if (init_result == 1 && old_value) {
				if (strlen(old_value) >= sizeof(old_password))
					first_error = ENAMETOOLONG;
				else {
					strcpy(old_password, old_value);
					old_password_present = 1;
				}
			} else if (init_result != 0) {
				first_error = EIO;
			}
		}
	}

	/* Publish count and credentials only after all metadata files exist. */
	if (first_error == 0) {
		count_attempted = 1;
		if (nvram_set_int("acc_num", acc_num + 1) != 0)
			first_error = errno ? errno : EIO;

		if (first_error == 0) {
			snprintf(nvram_name, sizeof(nvram_name), "acc_username%d", acc_num);
			username_attempted = 1;
			if (nvram_set(nvram_name, account) != 0)
				first_error = errno ? errno : EIO;
		}
		if (first_error == 0) {
			snprintf(password_key, sizeof(password_key), "acc_password%d", acc_num);
			password_attempted = 1;
			if (nvram_set(password_key, password) != 0)
				first_error = errno ? errno : EIO;
		}
		if (first_error == 0 && nvram_commit_safe() != 0)
			first_error = errno ? errno : EIO;
	}

	/* Any failed setter/commit restores the prior tail slot and account count. */
	if (first_error != 0 &&
	    (count_attempted || username_attempted || password_attempted)) {
		int restore_error = 0;
		int restore_result;

		if (password_attempted) {
			restore_result = old_password_present ?
				nvram_set(password_key, old_password) :
				nvram_unset(password_key);
			if (restore_result != 0)
				restore_error = errno ? errno : EIO;
		}
		if (username_attempted) {
			snprintf(nvram_name, sizeof(nvram_name), "acc_username%d", acc_num);
			restore_result = old_username_present ?
				nvram_set(nvram_name, old_username) :
				nvram_unset(nvram_name);
			if (restore_result != 0 && restore_error == 0)
				restore_error = errno ? errno : EIO;
		}
		if (count_attempted &&
		    nvram_set_int("acc_num", acc_num) != 0 && restore_error == 0)
			restore_error = errno ? errno : EIO;
		if (restore_error == 0 && nvram_commit_safe() != 0)
			restore_error = errno ? errno : EIO;
		if (restore_error != 0) {
			logmessage("disk_share",
				   "new account NVRAM rollback failed (errno %d)",
				   restore_error);
			first_error = EIO;
		}
	}

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
	free_disk_data(disk_list);

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
	int acc_num = 0, account_count, target;
	struct nvram_transaction transaction;
	char shifted_passwords[MAX_ACCOUNT_NUM][DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX];
	char **account_list = NULL;
	int result, i, first_error = 0, saved_errno;
	int service_result, service_error;
	char nvram_name[16];
	char var_file[256];

	if (!is_valid_account_name(account) || !strcmp(account, FTP_ANONYMOUS_USER))
		return -1;

	/* Keep the original inventory length for freeing after the NVRAM shift. */
	if (get_account_list(&acc_num, &account_list) != 0)
		return -1;
	if (acc_num <= 0) {
		free_2_dimension_list(&acc_num, &account_list);
		return -1;
	}
	account_count = acc_num;

	result = 0;
	for (i = 0; i < acc_num; ++i)
		if (!strcmp(account_list[i], account)) {
			result = 1;
			target = i;
			break;
		}
	if (result == 0) {
		free_2_dimension_list(&account_count, &account_list);
		return -1;
	}

	/*
	 * Snapshot all shifted passwords under one checked NVRAM lease before
	 * changing the account inventory. A backend read error must not erase a
	 * surviving account's password by collapsing it to an empty string.
	 */
	if (disk_share_account_password_shift_snapshot(
		    &disk_share_account_nvram_operations, &transaction,
		    account_count, target, MAX_ACCOUNT_NUM,
		    shifted_passwords) != 0) {
		saved_errno = errno ? errno : EIO;
		free_2_dimension_list(&account_count, &account_list);
		errno = saved_errno;
		return -1;
	}

	/* 2. remove the account from the persistent NVRAM inventory. */
	--acc_num;
	nvram_set_int("acc_num", acc_num);

	for (i = target; i < acc_num; ++i) {
		sprintf(nvram_name, "acc_username%d", i);
		nvram_set(nvram_name, account_list[i+1]);

		sprintf(nvram_name, "acc_password%d", i);
		nvram_set(nvram_name, shifted_passwords[i]);
	}

	/*
	 * 3. When no account remains, reset only modes that can be read reliably.
	 * A failed read must not be mistaken for an absent/zero setting.
	 */
	if (acc_num < 1) {
		char *mode_value = NULL;
		int samba_mode_default = 0, ftp_mode_default = 0;
		int samba_mode_status, ftp_mode_status, mode_reads_ok = 1;
		int mode_error;

		errno = 0;
		samba_mode_status = nvram_get_checked("st_samba_mode", &mode_value);
		if (disk_share_account_mode_needs_default(
			    samba_mode_status, mode_value, &samba_mode_default) != 0) {
			mode_error = errno ? errno : EIO;
			if (first_error == 0)
				first_error = mode_error;
			mode_reads_ok = 0;
		}

		mode_value = NULL;
		errno = 0;
		ftp_mode_status = nvram_get_checked("st_ftp_mode", &mode_value);
		if (disk_share_account_mode_needs_default(
			    ftp_mode_status, mode_value, &ftp_mode_default) != 0) {
			mode_error = errno ? errno : EIO;
			if (first_error == 0)
				first_error = mode_error;
			mode_reads_ok = 0;
		}

		if (mode_reads_ok && samba_mode_default &&
		    nvram_set_int("st_samba_mode", 1) != 0 && first_error == 0)
			first_error = errno ? errno : EIO;
		if (mode_reads_ok && ftp_mode_default &&
		    nvram_set_int("st_ftp_mode", 1) != 0 && first_error == 0)
			first_error = errno ? errno : EIO;
	}

	if (nvram_commit_safe() != 0 && first_error == 0)
		first_error = errno ? errno : EIO;

	/* Free with the pre-decrement count; account_list still owns that many strings. */
	free_2_dimension_list(&account_count, &account_list);

	/* 4. clean every volume in a complete inventory; preserve scan failures. */
	if (read_disk_data_checked(&disk_list) != 0) {
		if (first_error == 0)
			first_error = errno ? errno : EIO;
		disk_list = NULL;
	} else {
		for (follow_disk = disk_list; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
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
	}
	free_disk_data(disk_list);

	/* 6. reload services even when stale metadata could not be removed. */
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
	int i, acc_num = 0;
	char **account_list = NULL;
	char nvram_name[16], password_key[16], old_password[128];
	int account_order = -1, rename_required, password_update;
	char *old_password_value = NULL;
	int password_status, password_present = 0;
	char var_file[256], new_var_file[256];
	struct stat old_stat, new_stat;
	struct disk_share_account_rename_entry *moves = NULL, *moves_tail = NULL;
	struct disk_share_account_rename_entry *move, *next_move;
	int first_error = 0, saved_errno, rollback_error = 0;
	int service_result, service_error;
	int username_attempted = 0, password_attempted = 0;

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

	/* 1. check whether the account exists and the new name is available. */
	if (get_account_list(&acc_num, &account_list) != 0)
		return -1;

	for (i = 0; i < acc_num; ++i) {
		if (new_account != NULL && strcmp(new_account, account) &&
		    strlen(new_account) > 0 && !strcmp(account_list[i], new_account)) {
			free_2_dimension_list(&acc_num, &account_list);
			return -1;
		}
		if (!strcmp(account, account_list[i]))
			account_order = i;
	}
	free_2_dimension_list(&acc_num, &account_list);

	if (account_order == -1)
		return -1;
	if (new_account && *new_account && !strcmp(new_account, FTP_ANONYMOUS_USER))
		return -1;

	rename_required = new_account != NULL && *new_account &&
		strcmp(new_account, account) != 0;
	password_update = new_password != NULL && *new_password;

	/*
	 * Preserve the old password before attempting any mutation. A checked
	 * read error or overlong cached value must not turn rollback into an
	 * accidental empty-password write.
	 */
	if (password_update) {
		snprintf(password_key, sizeof(password_key), "acc_password%d", account_order);
		errno = 0;
		password_status = nvram_get_checked(password_key, &old_password_value);
		if (password_status < 0) {
			first_error = errno ? errno : EIO;
		} else if (password_status == 1 && old_password_value) {
			if (strlen(old_password_value) >= sizeof(old_password)) {
				first_error = ENAMETOOLONG;
			} else {
				strcpy(old_password, old_password_value);
				password_present = 1;
			}
		} else if (password_status != 0) {
			first_error = EIO;
		}
	}

	/*
	 * For a rename, preflight and publish all filesystem metadata first.
	 * NVRAM is changed only after every volume has accepted the new name;
	 * a later NVRAM failure compensates this complete batch.
	 */
	if (first_error == 0 && rename_required) {
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

	if (first_error == 0 && rename_required && moves &&
	    disk_share_account_rename_entries_apply(
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
	}

	/* Publish account identity/password only after the volume batch succeeds. */
	if (first_error == 0) {
		if (rename_required) {
			snprintf(nvram_name, sizeof(nvram_name),
				 "acc_username%d", account_order);
			username_attempted = 1;
			if (nvram_set(nvram_name, new_account) != 0)
				first_error = errno ? errno : EIO;
		}
		if (first_error == 0 && password_update) {
			password_attempted = 1;
			if (nvram_set(password_key, new_password) != 0)
				first_error = errno ? errno : EIO;
		}
		if (first_error == 0 && nvram_commit_safe() != 0)
			first_error = errno ? errno : EIO;
	}

	/* Restore NVRAM values and compensate file moves on any failed publish. */
	if (first_error != 0 && (username_attempted || password_attempted)) {
		int restore_error = 0;

		if (password_attempted) {
			int restore_result;

			restore_result = password_present ?
				nvram_set(password_key, old_password) :
				nvram_unset(password_key);
			if (restore_result != 0)
				restore_error = errno ? errno : EIO;
		}
		if (username_attempted) {
			snprintf(nvram_name, sizeof(nvram_name),
				 "acc_username%d", account_order);
			if (nvram_set(nvram_name, account) != 0 && restore_error == 0)
				restore_error = errno ? errno : EIO;
		}
		if (restore_error == 0 && nvram_commit_safe() != 0)
			restore_error = errno ? errno : EIO;
		if (restore_error != 0) {
			logmessage("disk_share",
				   "account NVRAM rollback failed (errno %d)",
				   restore_error);
			first_error = EIO;
		}
	}

	if (first_error != 0 && moves) {
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
	free_disk_data(disk_list);

	/* Reload services after both stores have reached their final state. */
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

int add_folder(const char *const mount_path, const char *const folder)
{
	int result, saved_errno;
	size_t mount_len, folder_len, full_len;
	char *full_path;
	mode_t old_umask;

	if (!mount_path || !*mount_path || !is_valid_folder_name(folder)) {
		errno = EINVAL;
		return -1;
	}

	/* A failed folder-list read is an error, not evidence of a free name. */
	result = test_if_exist_folder_in_mount_path(mount_path, folder);
	if (result != 0) {
		if (result > 0)
			errno = EEXIST;
		return -1;
	}

	mount_len = strlen(mount_path);
	folder_len = strlen(folder);
	if (mount_len > (size_t)-1 - 2 ||
	    folder_len > (size_t)-1 - mount_len - 2) {
		errno = ENAMETOOLONG;
		return -1;
	}
	full_len = mount_len + 1 + folder_len;
	full_path = (char *)malloc(full_len + 1);
	if (!full_path)
		return -1;
	memcpy(full_path, mount_path, mount_len);
	full_path[mount_len] = '/';
	memcpy(full_path + mount_len + 1, folder, folder_len + 1);

	old_umask = umask(0000);
	result = mkdir(full_path, 0777);
	(void)umask(old_umask);
	if (result != 0) {
		saved_errno = errno ? errno : EIO;
		free(full_path);
		errno = saved_errno;
		return -1;
	}

	/* Publish the new directory in the folder snapshot before permissions. */
	if (initial_folder_list_in_mount_path(mount_path) != 0) {
		saved_errno = errno ? errno : EIO;
		(void)rmdir(full_path);
		free(full_path);
		errno = saved_errno;
		return -1;
	}

	/* Checked path initializes missing files and atomically updates all users. */
	if (create_if_no_var_files(mount_path) != 0) {
		saved_errno = errno ? errno : EIO;
		free(full_path);
		errno = saved_errno;
		return -1;
	}

	free(full_path);
	return 0;
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

	var_info = read_whole_file(var_file);
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

int del_folder(const char *const mount_path, const char *const folder)
{
	int result, i, saved_errno, first_error = 0;
	int acc_num = 0;
	char **account_list = NULL;
	char var_file[256];
	char *target = NULL, *full_path = NULL;
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

	/* Preflight the complete inventory and every path before rmdir(). */
	result = get_account_list(&acc_num, &account_list);
	if (result != 0)
		goto failed;
	for (i = 0; i < acc_num; ++i) {
		if (disk_share_format_account_var_path(var_file, sizeof(var_file),
		                                       mount_path, account_list[i]) != 0)
			goto failed;
	}
	if (disk_share_format_account_var_path(var_file, sizeof(var_file),
	                                       mount_path, FTP_ANONYMOUS_USER) != 0)
		goto failed;

	result = test_if_exist_folder_in_mount_path(mount_path, folder);
	if (result < 0)
		goto failed;
	if (result == 0 && test_if_dir(full_path) != 1) {
		errno = ENOENT;
		goto failed;
	}
	/* Do not revoke access when rmdir fails (for example, a non-empty folder). */
	if (rmdir(full_path) != 0)
		goto failed;

	/* Report snapshot errors, but continue independent permission cleanup. */
	if (initial_folder_list_in_mount_path(mount_path) != 0)
		first_error = errno ? errno : EIO;

	for (i = 0; i < acc_num; ++i) {
		if (disk_share_update_folder_permission_file(
			    mount_path, account_list[i], target, NULL) != 0 &&
		    first_error == 0)
			first_error = errno ? errno : EIO;
	}
	if (disk_share_update_folder_permission_file(
		    mount_path, FTP_ANONYMOUS_USER, target, NULL) != 0 &&
	    first_error == 0)
		first_error = errno ? errno : EIO;

	if (first_error != 0) {
		errno = first_error;
		goto failed;
	}

	free(target);
	free(full_path);
	free_2_dimension_list(&acc_num, &account_list);
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	free(target);
	free(full_path);
	free_2_dimension_list(&acc_num, &account_list);
	errno = saved_errno;
	return -1;
}


int mod_folder(const char *const mount_path, const char *const folder, const char *const new_folder)
{
	int result, i, saved_errno, first_error = 0;
	int acc_num = 0;
	char **account_list = NULL;
	char var_file[256];
	char *target = NULL, *new_target = NULL;
	char *full_path = NULL, *new_full_path = NULL;
	struct stat target_stat;
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

	if (test_if_dir(full_path) != 1) {
		errno = ENOENT;
		goto failed;
	}
	result = test_if_exist_folder_in_mount_path(mount_path, folder);
	if (result < 0)
		goto failed;
	if (result == 0 && add_folder(mount_path, folder) != 0)
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

	result = get_account_list(&acc_num, &account_list);
	if (result != 0)
		goto failed;
	for (i = 0; i < acc_num; ++i) {
		if (disk_share_format_account_var_path(var_file, sizeof(var_file),
		                                       mount_path, account_list[i]) != 0)
			goto failed;
	}
	if (disk_share_format_account_var_path(var_file, sizeof(var_file),
	                                       mount_path, FTP_ANONYMOUS_USER) != 0)
		goto failed;

	if (rename(full_path, new_full_path) != 0)
		goto failed;
	if (initial_folder_list_in_mount_path(mount_path) != 0) {
		saved_errno = errno ? errno : EIO;
		if (rename(new_full_path, full_path) != 0)
			logmessage("disk_share", "cannot roll back folder rename after list publication failure");
		errno = saved_errno;
		goto failed;
	}
	free(full_path);
	full_path = NULL;
	free(new_full_path);
	new_full_path = NULL;

	/* Keep processing independent accounts even when one file is damaged. */
	for (i = 0; i < acc_num; ++i) {
		if (disk_share_update_folder_permission_file(
			    mount_path, account_list[i], target, new_target) != 0 &&
		    first_error == 0)
			first_error = errno ? errno : EIO;
	}
	if (disk_share_update_folder_permission_file(
		    mount_path, FTP_ANONYMOUS_USER, target, new_target) != 0 &&
	    first_error == 0)
		first_error = errno ? errno : EIO;

	if (first_error != 0) {
		errno = first_error;
		goto failed;
	}

	free(target);
	free(new_target);
	free_2_dimension_list(&acc_num, &account_list);
	return 0;

unchanged:
	free(full_path);
	free(new_full_path);
	free_2_dimension_list(&acc_num, &account_list);
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	free(target);
	free(new_target);
	free(full_path);
	free(new_full_path);
	free_2_dimension_list(&acc_num, &account_list);
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


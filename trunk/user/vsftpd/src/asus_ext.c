
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include "asus_ext.h"
#include "ftp_acl_path_policy.h"
#include "session.h"
#include "ls.h"
#include "tunables.h"
#include "defs.h"
#include "str.h"
#include "sysstr.h"
#include "sysutil.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <nvram_linux.h>
#include <disk_io_tools.h>
#include <disk_initial.h>
#include <disk_share.h>

#define HIDE_PARTITION_FILE ".hidden"

int asus_share_mode_read(void)
{
	return nvram_get_int("st_ftp_mode");
}

int test_if_dir_stat(const char *path)
{
	struct stat st;
	return (stat(path, &st) == 0) && (S_ISDIR(st.st_mode));
}

int test_if_hidden_share(const char *share_path)
{
	struct stat st;
	static char s_share_test[256];

	snprintf(s_share_test, sizeof(s_share_test), "%s/%s", share_path, HIDE_PARTITION_FILE);

	return (stat(s_share_test, &st) == 0) && (S_ISREG(st.st_mode));
}

int get_mount_layer(const char *basedir, char **mount_path, char **share)
{
	const char *component, *separator;
	size_t path_len, component_len, prefix_len, share_len;
	char *follow_info_end;
	int layer = BASE_LAYER;

	if (!basedir || !mount_path || !share || basedir[0] != '/')
		return -1;
	*mount_path = NULL;
	*share = NULL;

	path_len = strlen(basedir);
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

	if (!strcmp(basedir, "/"))
		return layer;

	if (path_len > 1)
		layer++;

	follow_info_end = NULL;
	component = basedir;
	while (*component &&
	       (component = index(component + 1, '/')) != NULL)
		++layer;

	if (layer >= MOUNT_LAYER) {
		separator = index(basedir + 1, '/');
		if (layer >= SHARE_LAYER && !separator)
			return -1;
		prefix_len = separator ? (size_t)(separator - basedir) : path_len;

		*mount_path = (char *)malloc(prefix_len + 1);
		if (!*mount_path)
			return -1;
		memcpy(*mount_path, basedir, prefix_len);
		(*mount_path)[prefix_len] = '\0';

		if (layer >= SHARE_LAYER) {
			component = separator + 1;
			follow_info_end = (char *)index(component, '/');
			share_len = follow_info_end ?
				(size_t)(follow_info_end - component) : strlen(component);
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
			memcpy(*share, component, share_len);
			(*share)[share_len] = '\0';
		}
	}

	return layer;
}
char* read_file_to_buffer(const char *target_file)
{
	FILE *fp;
	char *buffer;
	struct stat stat_buf;
	int i_readed, i_chunk, i_readed_total;

	memset(&stat_buf, 0, sizeof(stat_buf));
	stat(target_file, &stat_buf);

	fp = fopen(target_file, "r");
	if (!fp)
		return NULL;

	if (stat_buf.st_size < 1 || stat_buf.st_size > 16*1024*1024) {
		fclose(fp);
		return NULL;
	}

	buffer = (char *)malloc(stat_buf.st_size + 1);
	if (!buffer) {
		fclose(fp);
		return NULL;
	}

	i_chunk = MIN(1024, stat_buf.st_size);
	i_readed_total = 0;
	while ((i_readed = fread(buffer+i_readed_total, sizeof(char), i_chunk, fp)) > 0) {
		i_readed_total += i_readed;
		i_chunk = MIN(1024, stat_buf.st_size - i_readed_total);
		if (i_chunk < 1)
			break;
	}

	fclose(fp);

	buffer[i_readed_total] = 0;

	return buffer;
}

int get_permission_ftp(const char *account, const char *mount_path, const char *folder)
{
	char var_file[256], *var_info;
	char *target, *follow_info;
	int len, result;

	if (!account || !mount_path || !*mount_path || !folder || !*folder)
		return -1;

	snprintf(var_file, sizeof(var_file), "%s/.__%s_var.txt", mount_path, account);
	var_info = read_file_to_buffer(var_file);
	if (!var_info)
		return 0;

	len = strlen(folder)+strlen("*=");
	target = (char *)malloc(len+1);
	if (!target) {
		free(var_info);
		return -1;
	}
	sprintf(target, "*%s=", folder);

	follow_info = strstr(var_info, target);
	free(target);

	if (!follow_info) {
		free(var_info);
		return -1;
	}

	follow_info += len;
	if (strlen(follow_info) < 4 || follow_info[3] != '\n' ||
	    follow_info[0] < '0' || follow_info[0] > '3' ||
	    follow_info[1] < '0' || follow_info[1] > '3' ||
	    follow_info[2] < '0' || follow_info[2] > '3') {
		free(var_info);
		return -1;
	}

	result = follow_info[1]-'0';
	free(var_info);

	if (result < 0 || result > 3)
		return -1;

	return result;
}

static int
asus_check_permission_at(struct vsf_session* p_sess, int perm,
                         const char *directory)
{
	int i_layer, i_user_right, i_result;
	char *mount_path = NULL, *share_name = NULL;
	if (!tunable_chroot_local_user)
		return 1;
	if (!p_sess || !directory || directory[0] != '/')
		return 0;
	i_result = 1;
	i_layer = get_mount_layer(directory, &mount_path, &share_name);
	if (i_layer < 0) {
		i_result = 0;
		goto free_and_exit;
	}
	if (mount_path && test_if_hidden_share(mount_path)) {
		i_result = 0;
		goto free_and_exit;
	}
	if (i_layer < SHARE_LAYER) {
		if (perm == PERM_DELETE || perm == PERM_WRITE) {
			if (i_layer < MOUNT_LAYER || !p_sess->is_anonymous ||
			    p_sess->st_ftp_mode != 1) {
				i_result = 0;
				goto free_and_exit;
			}
		}
	} else {
		const char *account_ftp;
		if (p_sess->is_anonymous) {
			if (p_sess->st_ftp_mode == 1)
				goto free_and_exit;
			account_ftp = FTP_ANONYMOUS_USER;
		} else {
			account_ftp = str_getbuf(&p_sess->user_str);
		}
		i_user_right = get_permission_ftp(account_ftp, mount_path, share_name);
		if (perm == PERM_DELETE) {
			if (i_user_right < 3) {
				i_result = 0;
				goto free_and_exit;
			}
		} else if (perm == PERM_WRITE) {
			if (i_user_right < 2) {
				i_result = 0;
				goto free_and_exit;
			}
		} else if (perm == PERM_READ) {
			if (i_user_right < 1 || i_user_right == 2) {
				i_result = 0;
				goto free_and_exit;
			}
		}
	}
free_and_exit:
	if (mount_path) free(mount_path);
	if (share_name) free(share_name);
	return i_result;
}
int
asus_check_permission(struct vsf_session* p_sess, int perm)
{
	if (!tunable_chroot_local_user) return 1;
	if (!p_sess) return 0;
	str_getcwd(&p_sess->cwd_str);
	return asus_check_permission_at(p_sess, perm,
					str_getbuf(&p_sess->cwd_str));
}
int
asus_check_permission_path(struct vsf_session* p_sess, int perm,
                           const struct mystr* p_path_str)
{
	char parent[PATH_MAX], resolved[PATH_MAX];
	if (!p_path_str ||
	    ftp_acl_path_is_internal_metadata(str_getbuf(p_path_str)))
		return 0;
	if (!tunable_chroot_local_user) return 1;
	if (!p_sess ||
	    ftp_acl_path_resolve_parent(str_getbuf(p_path_str), parent,
					sizeof(parent), resolved,
					sizeof(resolved)) != 0)
		return 0;
	return asus_check_permission_at(p_sess, perm, resolved);
}

int
asus_check_permission_existing_path(struct vsf_session* p_sess, int perm,
                                    const struct mystr* p_path_str)
{
	char resolved_parent[PATH_MAX];
	if (!p_path_str ||
	    ftp_acl_path_is_internal_metadata(str_getbuf(p_path_str)))
		return 0;
	if (!tunable_chroot_local_user) return 1;
	if (!p_sess ||
	    ftp_acl_path_resolve_existing_parent(str_getbuf(p_path_str),
					 resolved_parent,
					 sizeof(resolved_parent)) != 0)
		return 0;
	return asus_check_permission_at(p_sess, perm, resolved_parent);
}

int
asus_check_permission_upload(struct vsf_session* p_sess,
                             const struct mystr* p_path_str)
{
	char resolved_parent[PATH_MAX];
	if (!p_path_str ||
	    ftp_acl_path_is_internal_metadata(str_getbuf(p_path_str)))
		return 0;
	if (!tunable_chroot_local_user) return 1;
	if (!p_sess ||
	    ftp_acl_path_resolve_write_parent(str_getbuf(p_path_str),
					 resolved_parent,
					 sizeof(resolved_parent)) != 0)
		return 0;
	return asus_check_permission_at(p_sess, PERM_WRITE, resolved_parent);
}


/*
 * Resolve and pin the authorized existing target before opening it. The
 * directory identity is checked again after the ACL lookup, so a concurrent
 * parent rename cannot redirect the final open to another share.
 */

/*
 * Authorize and remove an entry relative to a pinned canonical parent.
 * Final symlinks are deliberately not resolved: DELE removes the link entry,
 * while RMD uses AT_REMOVEDIR and therefore refuses a symlink.
 */

/*
 * Authorize and create a directory relative to a pinned canonical parent.
 * The mount/share ACL is evaluated against the canonical parent, not the
 * newly-created directory, matching the legacy MKD permission semantics.
 */
int
asus_make_directory_path(struct vsf_session* p_sess,
                         const struct mystr* p_path_str, unsigned int mode,
                         int* p_permission_denied)
{
  char lexical_parent[PATH_MAX], resolved_parent[PATH_MAX], leaf[PATH_MAX];
  int parent_fd, retval, saved_errno;
  if (!p_sess || !p_path_str || !p_permission_denied) {
    errno = EINVAL;
    return -1;
  }
  *p_permission_denied = 0;
  if (ftp_acl_path_is_internal_metadata(str_getbuf(p_path_str))) {
    errno = EACCES;
    *p_permission_denied = 1;
    return -1;
  }
  if (ftp_acl_path_split_entry(str_getbuf(p_path_str),
                               lexical_parent, sizeof(lexical_parent),
                               leaf, sizeof(leaf)) != 0) {
    errno = EINVAL;
    return -1;
  }
  if (ftp_acl_path_resolve_parent(str_getbuf(p_path_str),
                                  lexical_parent, sizeof(lexical_parent),
                                  resolved_parent, sizeof(resolved_parent)) != 0) {
    *p_permission_denied = 1;
    return -1;
  }
  parent_fd = ftp_acl_path_open_verified_directory(resolved_parent);
  if (parent_fd < 0) {
    *p_permission_denied = 1;
    return -1;
  }
  if (!asus_check_permission_at(p_sess, PERM_WRITE, resolved_parent)) {
    close(parent_fd);
    errno = EACCES;
    *p_permission_denied = 1;
    return -1;
  }
  if (ftp_acl_path_verify_directory_identity(resolved_parent, parent_fd) != 0) {
    saved_errno = errno;
    close(parent_fd);
    errno = saved_errno;
    *p_permission_denied = 1;
    return -1;
  }
  retval = ftp_acl_path_make_directory_at(parent_fd, leaf, mode);
  saved_errno = errno;
  close(parent_fd);
  errno = saved_errno;
  return retval;
}

int
asus_remove_path(struct vsf_session* p_sess, const struct mystr* p_path_str,
                 int remove_directory, int* p_permission_denied)
{
  char lexical_parent[PATH_MAX], resolved_parent[PATH_MAX], leaf[PATH_MAX];
  const char* path;
  unsigned int path_length;
  int parent_fd, retval, saved_errno;
  if (!p_sess || !p_path_str || !p_permission_denied) {
    errno = EINVAL;
    return -1;
  }
  *p_permission_denied = 0;
  path = str_getbuf(p_path_str);
  path_length = str_getlen(p_path_str);
  if (!remove_directory && path_length > 0 && path[path_length - 1] == '/') {
    errno = ENOTDIR;
    return -1;
  }
  if (ftp_acl_path_is_internal_metadata(path)) {
    errno = EACCES;
    *p_permission_denied = 1;
    return -1;
  }
  if (ftp_acl_path_split_entry(path, lexical_parent, sizeof(lexical_parent),
                               leaf, sizeof(leaf)) != 0) {
    errno = EINVAL;
    return -1;
  }
  if (ftp_acl_path_resolve_parent(path, lexical_parent,
                                  sizeof(lexical_parent), resolved_parent,
                                  sizeof(resolved_parent)) != 0) {
    *p_permission_denied = 1;
    return -1;
  }
  parent_fd = ftp_acl_path_open_verified_directory(resolved_parent);
  if (parent_fd < 0) {
    *p_permission_denied = 1;
    return -1;
  }
  if (!asus_check_permission_at(p_sess, PERM_DELETE, resolved_parent)) {
    close(parent_fd);
    errno = EACCES;
    *p_permission_denied = 1;
    return -1;
  }
  if (ftp_acl_path_verify_directory_identity(resolved_parent, parent_fd) != 0) {
    saved_errno = errno;
    close(parent_fd);
    errno = saved_errno;
    *p_permission_denied = 1;
    return -1;
  }
  retval = ftp_acl_path_remove_entry(parent_fd, leaf, remove_directory);
  saved_errno = errno;
  close(parent_fd);
  errno = saved_errno;
  return retval;
}

int
asus_rename_path(struct vsf_session* p_sess,
                 const struct mystr* p_source_path_str,
                 const struct mystr* p_destination_path_str,
                 int* p_permission_denied)
{
  char source_parent[PATH_MAX], source_leaf[PATH_MAX];
  char destination_parent[PATH_MAX], destination_leaf[PATH_MAX];
  int source_parent_fd = -1, destination_parent_fd = -1;
  int retval = -1, saved_errno;

  if (!p_sess || !p_source_path_str || !p_destination_path_str ||
      !p_permission_denied) {
    errno = EINVAL;
    return -1;
  }
  *p_permission_denied = 0;

  if (ftp_acl_path_resolve_rename_entry(str_getbuf(p_source_path_str),
                                        source_parent, sizeof(source_parent),
                                        source_leaf, sizeof(source_leaf)) != 0 ||
      ftp_acl_path_resolve_rename_entry(str_getbuf(p_destination_path_str),
                                        destination_parent,
                                        sizeof(destination_parent),
                                        destination_leaf,
                                        sizeof(destination_leaf)) != 0) {
    *p_permission_denied = 1;
    return -1;
  }

  source_parent_fd = ftp_acl_path_open_verified_directory(source_parent);
  if (source_parent_fd < 0) {
    *p_permission_denied = 1;
    return -1;
  }
  destination_parent_fd =
      ftp_acl_path_open_verified_directory(destination_parent);
  if (destination_parent_fd < 0) {
    saved_errno = errno;
    close(source_parent_fd);
    errno = saved_errno;
    *p_permission_denied = 1;
    return -1;
  }

  if (!asus_check_permission_at(p_sess, PERM_DELETE, source_parent) ||
      !asus_check_permission_at(p_sess, PERM_WRITE, destination_parent)) {
    errno = EACCES;
    *p_permission_denied = 1;
    goto rename_out;
  }
  if (ftp_acl_path_verify_directory_identity(source_parent,
                                              source_parent_fd) != 0 ||
      ftp_acl_path_verify_directory_identity(destination_parent,
                                              destination_parent_fd) != 0) {
    *p_permission_denied = 1;
    goto rename_out;
  }

  retval = ftp_acl_path_rename_entry(source_parent_fd, source_leaf,
                                     destination_parent_fd,
                                     destination_leaf);

rename_out:
  saved_errno = errno;
  close(destination_parent_fd);
  close(source_parent_fd);
  errno = saved_errno;
  return retval;
}

int
asus_open_existing_path(struct vsf_session* p_sess, int perm,
                        const struct mystr* p_path_str, int stat_only,
                        int* p_permission_denied)
{
  char resolved_parent[PATH_MAX], leaf[PATH_MAX];
  struct stat expected_parent, opened_parent;
  int parent_fd, file_fd, saved_errno;
  if (!p_permission_denied || !p_sess || !p_path_str) {
    errno = EINVAL;
    return -1;
  }
  *p_permission_denied = 0;
  if (ftp_acl_path_resolve_existing_entry(str_getbuf(p_path_str),
                                          resolved_parent,
                                          sizeof(resolved_parent),
                                          leaf, sizeof(leaf)) != 0) {
    *p_permission_denied = 1;
    return -1;
  }
  parent_fd = ftp_acl_path_open_verified_directory(resolved_parent);
  if (parent_fd < 0) {
    *p_permission_denied = 1;
    return -1;
  }
  if (!asus_check_permission_at(p_sess, perm, resolved_parent)) {
    close(parent_fd);
    errno = EACCES;
    *p_permission_denied = 1;
    return -1;
  }
  if (stat(resolved_parent, &expected_parent) != 0 ||
      fstat(parent_fd, &opened_parent) != 0) {
    saved_errno = errno;
    close(parent_fd);
    errno = saved_errno;
    *p_permission_denied = 1;
    return -1;
  }
  if (!S_ISDIR(opened_parent.st_mode) ||
      expected_parent.st_dev != opened_parent.st_dev ||
      expected_parent.st_ino != opened_parent.st_ino) {
    close(parent_fd);
    errno = ESTALE;
    *p_permission_denied = 1;
    return -1;
  }
  file_fd = ftp_acl_path_open_existing_file(parent_fd, leaf, stat_only);
  if (file_fd < 0) {
    saved_errno = errno;
    close(parent_fd);
    errno = saved_errno;
    return -1;
  }
  close(parent_fd);
  return file_fd;
}

int
asus_open_upload_path(struct vsf_session* p_sess,
                      const struct mystr* p_path_str, int exclusive,
                      unsigned int mode, int* p_parent_fd, char* p_leaf,
                      unsigned int leaf_size, int* p_cleanup_allowed)
{
  char resolved_parent[PATH_MAX];
  struct stat expected_parent, opened_parent;
  int parent_fd, file_fd, flags, saved_errno;
  if (!p_sess || !p_path_str || !p_parent_fd || !p_leaf ||
      leaf_size == 0 || !p_cleanup_allowed) {
    errno = EINVAL;
    return -1;
  }
  *p_parent_fd = -1;
  *p_cleanup_allowed = 0;
  p_leaf[0] = '\0';
  if (ftp_acl_path_resolve_write_entry(str_getbuf(p_path_str),
                                       resolved_parent,
                                       sizeof(resolved_parent),
                                       p_leaf, leaf_size,
                                       p_cleanup_allowed) != 0 ||
      !asus_check_permission_at(p_sess, PERM_WRITE, resolved_parent)) {
    errno = EACCES;
    return -1;
  }
  /*
   * Bind the pinned descriptor to the same directory object observed after
   * path resolution and ACL evaluation; fail closed on a directory swap.
   */
  if (stat(resolved_parent, &expected_parent) != 0)
    return -1;
  parent_fd = ftp_acl_path_open_directory(resolved_parent);
  if (parent_fd < 0)
    return -1;
  if (fstat(parent_fd, &opened_parent) != 0) {
    saved_errno = errno;
    close(parent_fd);
    errno = saved_errno;
    return -1;
  }
  if (!S_ISDIR(opened_parent.st_mode) ||
      expected_parent.st_dev != opened_parent.st_dev ||
      expected_parent.st_ino != opened_parent.st_ino) {
    close(parent_fd);
    errno = ESTALE;
    return -1;
  }
  flags = O_WRONLY | O_NONBLOCK | O_CREAT | O_NOFOLLOW | O_CLOEXEC;
  if (exclusive)
    flags |= O_EXCL | O_APPEND;
  file_fd = openat(parent_fd, p_leaf, flags, mode);
  if (file_fd < 0) {
    saved_errno = errno;
    close(parent_fd);
    errno = saved_errno;
    return -1;
  }
  *p_parent_fd = parent_fd;
  return file_fd;
}

int
asus_cleanup_failed_upload(int parent_fd, const char* leaf, int file_fd)
{
  return ftp_acl_path_unlink_opened_entry(parent_fd, leaf, file_fd);
}

int
asus_check_permission_directory(struct vsf_session* p_sess,
                                const struct mystr* p_path_str)
{
	char resolved[PATH_MAX];
	if (!p_path_str ||
	    ftp_acl_path_is_internal_metadata(str_getbuf(p_path_str)))
		return 0;
	if (!tunable_chroot_local_user) return 1;
	if (!p_sess ||
	    ftp_acl_path_resolve_directory(str_getbuf(p_path_str), resolved,
				   sizeof(resolved)) != 0)
		return 0;
	return asus_check_permission_at(p_sess, PERM_READ, resolved);
}

int
asus_check_permission_open_directory(struct vsf_session* p_sess,
                                     const struct mystr* p_path_str,
                                     int directory_fd)
{
  char resolved[PATH_MAX];
  if (!p_path_str || directory_fd < 0 ||
      ftp_acl_path_is_internal_metadata(str_getbuf(p_path_str)) ||
      ftp_acl_path_resolve_directory(str_getbuf(p_path_str), resolved,
                                     sizeof(resolved)) != 0 ||
      ftp_acl_path_verify_directory_identity(resolved, directory_fd) != 0)
    return 0;
  if (tunable_chroot_local_user &&
      (!p_sess || !asus_check_permission_at(p_sess, PERM_READ, resolved)))
    return 0;
  return ftp_acl_path_verify_directory_identity(resolved, directory_fd) == 0;
}

int
asus_open_subdirectory_path(struct vsf_session* p_sess, int parent_fd,
                            const struct mystr* p_parent_path_str,
                            const struct mystr* p_leaf_str,
                            int* p_permission_denied)
{
  char resolved_parent[PATH_MAX], child_path[PATH_MAX];
  const char* leaf;
  int child_fd, path_length, saved_errno;
  if (!p_sess || parent_fd < 0 || !p_parent_path_str || !p_leaf_str ||
      !p_permission_denied) {
    errno = EINVAL;
    return -1;
  }
  *p_permission_denied = 0;
  leaf = str_getbuf(p_leaf_str);
  if (!*leaf || strchr(leaf, '/') || strcmp(leaf, ".") == 0 ||
      strcmp(leaf, "..") == 0) {
    errno = EINVAL;
    return -1;
  }
  if (ftp_acl_path_is_internal_metadata(leaf)) {
    errno = EACCES;
    *p_permission_denied = 1;
    return -1;
  }
  if (ftp_acl_path_resolve_directory(str_getbuf(p_parent_path_str),
                                     resolved_parent,
                                     sizeof(resolved_parent)) != 0 ||
      ftp_acl_path_verify_directory_identity(resolved_parent, parent_fd) != 0) {
    *p_permission_denied = 1;
    return -1;
  }

  child_fd = ftp_acl_path_open_child_directory(parent_fd, leaf);
  if (child_fd < 0)
    return -1;

  if (strcmp(resolved_parent, "/") == 0)
    path_length = snprintf(child_path, sizeof(child_path), "/%s", leaf);
  else
    path_length = snprintf(child_path, sizeof(child_path), "%s/%s",
                           resolved_parent, leaf);
  if (path_length < 0 || (size_t) path_length >= sizeof(child_path)) {
    close(child_fd);
    errno = ENAMETOOLONG;
    return -1;
  }

  if (!asus_check_permission_at(p_sess, PERM_READ, child_path)) {
    close(child_fd);
    errno = EACCES;
    *p_permission_denied = 1;
    return -1;
  }
  if (ftp_acl_path_verify_directory_identity(resolved_parent, parent_fd) != 0 ||
      ftp_acl_path_verify_child_directory(parent_fd, leaf, child_fd) != 0) {
    saved_errno = errno;
    close(child_fd);
    errno = saved_errno;
    *p_permission_denied = 1;
    return -1;
  }
  return child_fd;
}

int
asus_resolve_listing_base(const struct mystr* p_base_dir_str, int directory_fd,
                          char* resolved_base, size_t resolved_size)
{
  if (!p_base_dir_str || directory_fd < 0 || !resolved_base ||
      ftp_acl_path_resolve_directory(str_getbuf(p_base_dir_str), resolved_base,
                                     resolved_size) != 0 ||
      ftp_acl_path_verify_directory_identity(resolved_base, directory_fd) != 0)
    return -1;
  return 0;
}

int
asus_check_file_visible(struct vsf_session* p_sess,
                        const char* resolved_base, int directory_fd,
                        const struct mystr* p_filename_str)
{
  int i_layer, i_user_right, i_result;
  char fullpath[PATH_MAX];
  char *mount_path = NULL, *share_name = NULL;
  const char *p_filename;
  struct stat entry_stat;

  if (!p_filename_str || str_isempty(p_filename_str))
    return 0;
  p_filename = str_getbuf(p_filename_str);
  if (ftp_acl_path_is_internal_metadata(p_filename))
    return 0;
  if (!tunable_chroot_local_user)
    return 1;
  if (!p_sess || directory_fd < 0 || !resolved_base || resolved_base[0] != '/')
    return 0;
  if (!strcmp(p_filename, ".") || !strcmp(p_filename, ".."))
    return 1;

  if (ftp_acl_path_build_child_path(resolved_base, p_filename, fullpath,
                                    sizeof(fullpath)) != 0)
    return 0;

  i_result = 1;
  i_layer = get_mount_layer(fullpath, &mount_path, &share_name);
  if (i_layer < 0) {
    i_result = 0;
    goto free_and_exit;
  }
  if (mount_path && test_if_hidden_share(mount_path)) {
    i_result = 0;
    goto free_and_exit;
  }

  if (i_layer == MOUNT_LAYER) {
    if (fstatat(directory_fd, p_filename, &entry_stat, 0) != 0 ||
        !S_ISDIR(entry_stat.st_mode)) {
      i_result = 0;
      goto free_and_exit;
    }
  } else if (i_layer == SHARE_LAYER) {
    const char *account_ftp;
    if (test_if_System_folder(p_filename) || !strncmp(p_filename, ".__", 3)) {
      i_result = 0;
      goto free_and_exit;
    }
    if (fstatat(directory_fd, p_filename, &entry_stat, 0) != 0 ||
        !S_ISDIR(entry_stat.st_mode)) {
      i_result = 0;
      goto free_and_exit;
    }
    if (p_sess->is_anonymous) {
      if (p_sess->st_ftp_mode == 1)
        goto free_and_exit;
      account_ftp = FTP_ANONYMOUS_USER;
    } else {
      account_ftp = str_getbuf(&p_sess->user_str);
    }
    i_user_right = get_permission_ftp(account_ftp, mount_path, share_name);
    if (i_user_right < 1) {
      i_result = 0;
      goto free_and_exit;
    }
  }

free_and_exit:
  if (mount_path)
    free(mount_path);
  if (share_name)
    free(share_name);
  return i_result;
}

struct passwd *asus_getpwnam(const char *name)
{
	static struct passwd resultbuf;
	static char pw_name[VSFTP_USERNAME_MAX];
	static char pw_passwd[VSFTP_PASSWORD_MAX];
	char nvram_value[32];
	char *tmp_user, *tmp_pass;
	int acc_num, i;

	if (!name)
		return NULL;

	if (strcmp(name, "root") == 0){
		strcpy(pw_name, name);
		strcpy(pw_passwd, "*");
		resultbuf.pw_name = pw_name;
		resultbuf.pw_passwd = pw_passwd;
		resultbuf.pw_uid = 0;
		resultbuf.pw_gid = 0;
		resultbuf.pw_gecos = pw_name;
		resultbuf.pw_dir = POOL_MOUNT_ROOT;
		resultbuf.pw_shell = "/bin/false";
		return &resultbuf;
	}
	else if(strcmp(name, "ftp") == 0){
		strcpy(pw_name, name);
		strcpy(pw_passwd, "*");
		resultbuf.pw_name = pw_name;
		resultbuf.pw_passwd = pw_passwd;
		resultbuf.pw_uid = 99; // nobody
		resultbuf.pw_gid = 99; // nogroup
		resultbuf.pw_gecos = pw_name;
		resultbuf.pw_dir = POOL_MOUNT_ROOT;
		resultbuf.pw_shell = "/bin/false";
		return &resultbuf;
	}
	else {
		acc_num = nvram_get_int("acc_num");
		if (acc_num > MAX_ACCOUNT_NUM) acc_num = MAX_ACCOUNT_NUM;
		for(i = 0; i < acc_num; i++){
			sprintf(nvram_value, "acc_username%d", i);
			tmp_user = nvram_safe_get(nvram_value);
			sprintf(nvram_value, "acc_password%d", i);
			tmp_pass = nvram_safe_get(nvram_value);
			if (strcmp(name, tmp_user) == 0) {
				if (strlen(name) >= sizeof(pw_name) ||
				    strlen(tmp_pass) >= sizeof(pw_passwd))
					return NULL;
				strcpy(pw_name, name);
				strcpy(pw_passwd, tmp_pass);
				resultbuf.pw_name = pw_name;
				resultbuf.pw_passwd = pw_passwd;
				resultbuf.pw_uid = 99; // nobody
				resultbuf.pw_gid = 99; // nogroup
				resultbuf.pw_gecos = pw_name;
				resultbuf.pw_dir = POOL_MOUNT_ROOT;
				resultbuf.pw_shell = "/bin/sh";
				return &resultbuf;
			}
		}
	}

	return NULL;
}

int
asus_check_auth(struct mystr* p_user_str, const struct mystr* p_pass_str)
{
	const char* p_pass;
	const struct passwd* p_pwd = asus_getpwnam(str_getbuf(p_user_str));
	if (!p_pwd)
	{
		return 0;
	}
	p_pass = str_getbuf(p_pass_str);
	if ( vsf_sysutil_strcmp(p_pass, "*") &&
	    !vsf_sysutil_strcmp(p_pass, p_pwd->pw_passwd))
	{
		return 1;
	}
	return 0;
}


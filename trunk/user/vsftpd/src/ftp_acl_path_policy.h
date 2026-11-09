#ifndef FTP_ACL_PATH_POLICY_H
#define FTP_ACL_PATH_POLICY_H
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
static int
ftp_acl_path_parent(const char *path, char *parent, size_t parent_size)
{
  size_t length, parent_length;
  if (!path || !*path || !parent || parent_size == 0)
    return -1;
  length = strlen(path);
  while (length > 1 && path[length - 1] == '/')
    --length;
  if (length == 1 && path[0] == '/') {
    if (parent_size < 2) return -1;
    parent[0] = '/'; parent[1] = '\0'; return 0;
  }
  parent_length = length;
  while (parent_length > 0 && path[parent_length - 1] != '/')
    --parent_length;
  if (parent_length == 0) {
    if (parent_size < 2) return -1;
    parent[0] = '.'; parent[1] = '\0'; return 0;
  }
  if (parent_length > 1) --parent_length;
  if (parent_length >= parent_size) return -1;
  memcpy(parent, path, parent_length);
  parent[parent_length] = '\0';
  return 0;
}
static int
ftp_acl_path_resolve_parent(const char *path, char *parent,
                            size_t parent_size, char *resolved,
                            size_t resolved_size)
{
  if (!resolved || resolved_size < PATH_MAX ||
      ftp_acl_path_parent(path, parent, parent_size) != 0)
    return -1;
  if (realpath(parent, resolved) == NULL)
    return -1;
  return 0;
}


/*
 * These files hold the USB mount's account/share ACL database. They are
 * implementation metadata, not user content, and must never be exposed by
 * direct FTP file commands.
 */
static int
ftp_acl_path_is_internal_metadata(const char *path)
{
  const char *base;
  size_t length;
  if (!path || !*path)
    return 0;
  base = strrchr(path, '/');
  base = base ? base + 1 : path;
  if (strcmp(base, ".__folder_list.txt") == 0)
    return 1;
  length = strlen(base);
  return length > 11 && strncmp(base, ".__", 3) == 0 &&
         strcmp(base + length - 8, "_var.txt") == 0;
}

static int
ftp_acl_path_resolve_existing_parent(const char *path, char *resolved_parent,
                                     size_t resolved_size)
{
  char target[PATH_MAX], parent[PATH_MAX];
  if (!path || !*path || !resolved_parent || resolved_size < PATH_MAX)
    return -1;
  if (realpath(path, target) == NULL ||
      ftp_acl_path_is_internal_metadata(target))
    return -1;
  if (ftp_acl_path_parent(target, parent, sizeof(parent)) != 0)
    return -1;
  if (realpath(parent, resolved_parent) == NULL)
    return -1;
  return 0;
}

/*
 * str_create() follows an existing final symlink. Resolve it before checking
 * the share ACL; reject dangling or otherwise unresolvable existing entries.
 * A genuinely absent leaf is checked against its canonical parent instead.
 */
static int
ftp_acl_path_resolve_write_parent(const char *path, char *resolved_parent,
                                  size_t resolved_size)
{
  char target[PATH_MAX], parent[PATH_MAX];
  struct stat st;
  if (!path || !*path || !resolved_parent || resolved_size < PATH_MAX)
    return -1;
  if (realpath(path, target) != NULL) {
    if (ftp_acl_path_is_internal_metadata(target))
      return -1;
    if (ftp_acl_path_parent(target, parent, sizeof(parent)) != 0)
      return -1;
    if (realpath(parent, resolved_parent) == NULL)
      return -1;
    return 0;
  }
  if (lstat(path, &st) == 0 || errno != ENOENT)
    return -1;
  return ftp_acl_path_resolve_parent(path, parent, sizeof(parent),
                                     resolved_parent, resolved_size);
}


/*
 * Split a file entry into its parent and final component. The leaf is kept
 * separate so subsequent operations can be relative to an opened directory.
 */
static int
ftp_acl_path_split_entry(const char *path, char *parent, size_t parent_size,
                         char *leaf, size_t leaf_size)
{
  size_t length, base_offset, base_length, parent_length;
  const char *slash;
  if (!path || !*path || !parent || !leaf ||
      parent_size == 0 || leaf_size == 0)
    return -1;
  length = strlen(path);
  if (length >= PATH_MAX)
    return -1;
  while (length > 1 && path[length - 1] == '/')
    --length;
  if (length == 1 && path[0] == '/')
    return -1;
  slash = NULL;
  {
    size_t index = length;
    while (index > 0) {
      --index;
      if (path[index] == '/') {
        slash = path + index;
        break;
      }
    }
  }
  if (!slash) {
    base_offset = 0;
    parent_length = 1;
    if (parent_size < 2)
      return -1;
    parent[0] = '.';
    parent[1] = '\0';
  } else {
    base_offset = (size_t)(slash - path) + 1;
    parent_length = (slash == path) ? 1 : (size_t)(slash - path);
    if (parent_length >= parent_size)
      return -1;
    memcpy(parent, path, parent_length);
    parent[parent_length] = '\0';
  }
  base_length = length - base_offset;
  if (base_length == 0 || base_length >= leaf_size)
    return -1;
  if ((base_length == 1 && path[base_offset] == '.') ||
      (base_length == 2 && path[base_offset] == '.' &&
       path[base_offset + 1] == '.'))
    return -1;
  memcpy(leaf, path + base_offset, base_length);
  leaf[base_length] = '\0';
  return 0;
}

/*
 * Resolve an existing pathname through its final symlink and split the
 * canonical target into a directory and one leaf component. Later syscalls
 * use the returned parent descriptor and leaf, never the original pathname.
 */
static int
ftp_acl_path_resolve_existing_entry(const char *path, char *resolved_parent,
                                    size_t resolved_size, char *leaf,
                                    size_t leaf_size)
{
  char target[PATH_MAX], parent[PATH_MAX];
  if (!path || !*path || !resolved_parent || resolved_size < PATH_MAX ||
      !leaf || leaf_size == 0 || strlen(path) >= PATH_MAX ||
      ftp_acl_path_is_internal_metadata(path)) {
    errno = EACCES;
    return -1;
  }
  if (realpath(path, target) == NULL)
    return -1;
  if (ftp_acl_path_is_internal_metadata(target)) {
    errno = EACCES;
    return -1;
  }
  if (ftp_acl_path_split_entry(target, parent, sizeof(parent),
                               leaf, leaf_size) != 0 ||
      realpath(parent, resolved_parent) == NULL)
    return -1;
  return 0;
}

/*
 * Open an absolute canonical directory one component at a time. O_PATH keeps
 * this usable for write-only directories while providing a stable dirfd.
 * O_NOFOLLOW rejects a component substituted with a symlink after realpath().
 */
static int
ftp_acl_path_open_directory(const char *path)
{
  int dir_fd, next_fd;
  const char *component;
  char name[PATH_MAX];
  size_t path_length;
  if (!path || path[0] != '/')
    return -1;
  path_length = strlen(path);
  if (path_length == 0 || path_length >= PATH_MAX)
    return -1;
  dir_fd = open("/", O_PATH | O_DIRECTORY | O_CLOEXEC);
  if (dir_fd < 0)
    return -1;
  component = path + 1;
  while (*component) {
    const char *end = component;
    size_t name_length;
    while (*end && *end != '/')
      ++end;
    name_length = (size_t)(end - component);
    if (name_length == 0 || name_length >= sizeof(name) ||
        (name_length == 1 && component[0] == '.') ||
        (name_length == 2 && component[0] == '.' && component[1] == '.')) {
      close(dir_fd);
      errno = EINVAL;
      return -1;
    }
    memcpy(name, component, name_length);
    name[name_length] = '\0';
    next_fd = openat(dir_fd, name,
                     O_PATH | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (next_fd < 0) {
      int saved_errno = errno;
      close(dir_fd);
      errno = saved_errno;
      return -1;
    }
    close(dir_fd);
    dir_fd = next_fd;
    component = (*end == '/') ? end + 1 : end;
  }
  return dir_fd;
}

/*
 * Pin a canonical directory and verify that the opened descriptor still
 * names the directory resolved by the caller.
 */
static int
ftp_acl_path_open_verified_directory(const char *path)
{
  struct stat expected, opened;
  int dir_fd, saved_errno;
  if (!path || path[0] != '/' || stat(path, &expected) != 0)
    return -1;
  dir_fd = ftp_acl_path_open_directory(path);
  if (dir_fd < 0)
    return -1;
  if (fstat(dir_fd, &opened) != 0) {
    saved_errno = errno;
    close(dir_fd);
    errno = saved_errno;
    return -1;
  }
  if (!S_ISDIR(opened.st_mode) ||
      expected.st_dev != opened.st_dev ||
      expected.st_ino != opened.st_ino) {
    close(dir_fd);
    errno = ESTALE;
    return -1;
  }
  return dir_fd;
}

/*
 * Open only a regular file below a pinned directory. O_NOFOLLOW prevents a
 * final-component symlink substitution; comparing the directory entry with
 * the returned descriptor rejects a replacement between fstatat() and openat().
 * stat_only uses O_PATH so SIZE retains stat semantics without needing read
 * permission on the file itself.
 */
static int
ftp_acl_path_open_existing_file(int dir_fd, const char *leaf, int stat_only)
{
  struct stat expected, opened;
  int file_fd, flags, saved_errno;
  if (dir_fd < 0 || !leaf || !*leaf || strchr(leaf, '/') ||
      strcmp(leaf, ".") == 0 || strcmp(leaf, "..") == 0) {
    errno = EINVAL;
    return -1;
  }
  if (fstatat(dir_fd, leaf, &expected, AT_SYMLINK_NOFOLLOW) != 0)
    return -1;
  if (!S_ISREG(expected.st_mode)) {
    errno = EINVAL;
    return -1;
  }
  flags = (stat_only ? O_PATH : (O_RDONLY | O_NONBLOCK)) |
          O_NOFOLLOW | O_CLOEXEC;
  file_fd = openat(dir_fd, leaf, flags);
  if (file_fd < 0)
    return -1;
  if (fstat(file_fd, &opened) != 0) {
    saved_errno = errno;
    close(file_fd);
    errno = saved_errno;
    return -1;
  }
  if (!S_ISREG(opened.st_mode) ||
      expected.st_dev != opened.st_dev ||
      expected.st_ino != opened.st_ino) {
    close(file_fd);
    errno = ESTALE;
    return -1;
  }
  return file_fd;
}

/*
 * Resolve the effective upload target before splitting it. Existing final
 * symlinks retain legacy write-through behavior, but the canonical target
 * leaf is later opened with O_NOFOLLOW. Missing leaves are allowed only if
 * their parent resolves. Do not cleanup through a final symlink alias.
 */
static int
ftp_acl_path_resolve_write_entry(const char *path, char *resolved_parent,
                                 size_t resolved_size, char *leaf,
                                 size_t leaf_size, int *cleanup_allowed)
{
  char target[PATH_MAX], parent[PATH_MAX];
  struct stat st;
  int lstat_result, saved_errno;
  if (!path || !*path || !resolved_parent || resolved_size < PATH_MAX ||
      !leaf || leaf_size == 0 || !cleanup_allowed ||
      ftp_acl_path_is_internal_metadata(path) || strlen(path) >= PATH_MAX)
    return -1;
  *cleanup_allowed = 0;
  if (realpath(path, target) != NULL) {
    if (ftp_acl_path_is_internal_metadata(target) ||
        ftp_acl_path_split_entry(target, parent, sizeof(parent),
                                 leaf, leaf_size) != 0 ||
        realpath(parent, resolved_parent) == NULL)
      return -1;
    lstat_result = lstat(path, &st);
    if (lstat_result == 0 && !S_ISLNK(st.st_mode))
      *cleanup_allowed = 1;
    return 0;
  }
  saved_errno = errno;
  if (saved_errno != ENOENT || lstat(path, &st) == 0 || errno != ENOENT)
    return -1;
  if (ftp_acl_path_split_entry(path, parent, sizeof(parent),
                               leaf, leaf_size) != 0 ||
      realpath(parent, resolved_parent) == NULL)
    return -1;
  *cleanup_allowed = 1;
  return 0;
}

/*
 * This prevents cleanup from deleting a different inode that was already
 * installed at the upload leaf before the identity check. Linux 4.4 has no
 * unlink-by-file-descriptor operation, so the final fstatat()/unlinkat()
 * name pair remains best-effort against concurrent writers in the same dir.
 */
static int
ftp_acl_path_unlink_opened_entry(int dir_fd, const char *leaf, int file_fd)
{
  struct stat opened, current;
  if (dir_fd < 0 || !leaf || !*leaf || file_fd < 0 ||
      fstat(file_fd, &opened) != 0 ||
      !S_ISREG(opened.st_mode) ||
      fstatat(dir_fd, leaf, &current, AT_SYMLINK_NOFOLLOW) != 0 ||
      !S_ISREG(current.st_mode) ||
      opened.st_dev != current.st_dev || opened.st_ino != current.st_ino)
    return -1;
  return unlinkat(dir_fd, leaf, 0);
}

static int
ftp_acl_path_resolve_directory(const char *path, char *resolved,
                               size_t resolved_size)
{
  if (!path || !*path || !resolved || resolved_size < PATH_MAX)
    return -1;
  return realpath(path, resolved) == NULL ? -1 : 0;
}

#endif /* FTP_ACL_PATH_POLICY_H */

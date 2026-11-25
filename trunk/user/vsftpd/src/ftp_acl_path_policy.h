#ifndef FTP_ACL_PATH_POLICY_H
#define FTP_ACL_PATH_POLICY_H

/*
 * Share rights are 0=none, 1=read, 2=write, 3=read/write.
 * Unknown values fail closed. Keep directory access and LIST visibility
 * on the same read-permission rule.
 */
static int
ftp_acl_share_rights_allow_read(int user_right)
{
  return user_right == 1 || user_right == 3;
}
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <unistd.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#ifndef RENAME_NOREPLACE
#define RENAME_NOREPLACE (1U << 0)
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
static int ftp_acl_path_is_internal_metadata(const char *path);

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
  if (ftp_acl_path_is_internal_metadata(resolved)) {
    errno = EACCES;
    return -1;
  }
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
  const char *base, *component, *end;
  size_t length;
  static const char cleanup_prefix[] = ".__vsftpd_cleanup_";

  if (!path || !*path)
    return 0;
  component = path;
  while (*component) {
    while (*component == '/')
      ++component;
    if (!*component)
      break;
    end = component;
    while (*end && *end != '/')
      ++end;
    if ((size_t)(end - component) >= sizeof(cleanup_prefix) - 1U &&
        strncmp(component, cleanup_prefix,
                sizeof(cleanup_prefix) - 1U) == 0)
      return 1;
    component = end;
  }

  base = strrchr(path, '/');
  base = base ? base + 1 : path;
  if (strcmp(base, ".__folder_list.txt") == 0)
    return 1;
  length = strlen(base);
  return length > 11 && strncmp(base, ".__", 3) == 0 &&
         strcmp(base + length - 8, "_var.txt") == 0;
}

/*
 * The account name comes from the authenticated FTP USER field and is used
 * to select a protected ACL metadata file. Never let it add path components
 * or silently truncate the metadata path to a different filename.
 */
static int
ftp_acl_path_build_account_permission_path(const char *mount_path,
                                           const char *account,
                                           char *path, size_t path_size)
{
  int written;

  if (!mount_path || mount_path[0] != '/' || !account || !*account ||
      !path || path_size == 0)
  {
    errno = EINVAL;
    return -1;
  }
  if (strchr(account, '/') != NULL)
  {
    errno = EACCES;
    return -1;
  }
  written = snprintf(path, path_size, "%s/.__%s_var.txt",
                     mount_path, account);
  if (written < 0)
  {
    errno = EIO;
    return -1;
  }
  if ((size_t) written >= path_size)
  {
    errno = ENAMETOOLONG;
    return -1;
  }
  return 0;
}


/*
 * Resolve one share permission record without allowing a folder name to span
 * metadata lines. Folder names may contain arbitrary bytes other than NUL and
 * '/', including newlines; treating the file as independent records prevents
 * a crafted name from matching the next share's entry.
 */
static int
ftp_acl_path_share_permission(const char *contents, size_t contents_size,
                              const char *folder)
{
  const char *line, *end, *newline;
  size_t folder_size, line_size;
  int found = 0;
  int result = -1;

  if (!contents || !folder || !*folder ||
      strchr(folder, '\n') != NULL || strchr(folder, '\r') != NULL ||
      memchr(contents, '\0', contents_size) != NULL)
    return -1;

  folder_size = strlen(folder);
  if (folder_size > (size_t)-1 - 5U)
    return -1;

  line = contents;
  end = contents + contents_size;
  while (line < end)
  {
    newline = memchr(line, '\n', (size_t)(end - line));
    line_size = newline != NULL ?
                (size_t)(newline - line) : (size_t)(end - line);
    if (line_size >= folder_size + 2U &&
        line[0] == '*' &&
        line[folder_size + 1U] == '=' &&
        memcmp(line + 1, folder, folder_size) == 0)
    {
      /*
       * Once the exact key matches, a malformed or duplicate record must
       * invalidate the permission file rather than being skipped in favour
       * of a later well-formed record.
       */
      if (found || newline == NULL || line_size != folder_size + 5U ||
          line[folder_size + 2U] < '0' || line[folder_size + 2U] > '3' ||
          line[folder_size + 3U] < '0' || line[folder_size + 3U] > '3' ||
          line[folder_size + 4U] < '0' || line[folder_size + 4U] > '3')
        return -1;
      result = line[folder_size + 3U] - '0';
      found = 1;
    }
    if (newline == NULL)
      break;
    line = newline + 1;
  }
  return found ? result : -1;
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
  if (ftp_acl_path_is_internal_metadata(resolved_parent)) {
    errno = EACCES;
    return -1;
  }
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
 * Resolve a rename operand into a canonical parent and one leaf component.
 * Without a trailing slash the final component is intentionally not followed:
 * renameat() must move a symlink entry, not its target. A trailing slash has
 * directory-path semantics, so realpath() canonicalizes the existing target.
 */
static int
ftp_acl_path_resolve_rename_entry(const char *path, char *resolved_parent,
                                  size_t resolved_size, char *leaf,
                                  size_t leaf_size)
{
  char entry[PATH_MAX], parent[PATH_MAX];
  size_t length;
  int trailing_slash;

  if (!path || !*path || !resolved_parent || resolved_size < PATH_MAX ||
      !leaf || leaf_size == 0) {
    errno = EINVAL;
    return -1;
  }
  length = strlen(path);
  if (length >= PATH_MAX || ftp_acl_path_is_internal_metadata(path)) {
    errno = EACCES;
    return -1;
  }
  trailing_slash = path[length - 1] == '/';
  if (trailing_slash) {
    if (realpath(path, entry) == NULL)
      return -1;
    if (ftp_acl_path_is_internal_metadata(entry)) {
      errno = EACCES;
      return -1;
    }
    if (ftp_acl_path_split_entry(entry, parent, sizeof(parent),
                                 leaf, leaf_size) != 0)
      return -1;
  } else {
    if (ftp_acl_path_split_entry(path, parent, sizeof(parent),
                                 leaf, leaf_size) != 0)
      return -1;
    if (ftp_acl_path_is_internal_metadata(leaf)) {
      errno = EACCES;
      return -1;
    }
  }
  if (realpath(parent, resolved_parent) == NULL)
    return -1;
  if (ftp_acl_path_is_internal_metadata(resolved_parent)) {
    errno = EACCES;
    return -1;
  }
  return 0;
}

/* Operate on directory entries relative to the exact parent directories already authorized. */

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
 * Revalidate that a canonical parent pathname still names the pinned
 * directory after the share ACL check. This closes parent rename/symlink
 * substitution between authorization and unlinkat().
 */
static int
ftp_acl_path_verify_directory_identity(const char *path, int dir_fd)
{
  struct stat expected, opened;
  if (!path || dir_fd < 0) {
    errno = EINVAL;
    return -1;
  }
  if (stat(path, &expected) != 0 || fstat(dir_fd, &opened) != 0)
    return -1;
  if (!S_ISDIR(opened.st_mode) ||
      expected.st_dev != opened.st_dev ||
      expected.st_ino != opened.st_ino) {
    errno = ESTALE;
    return -1;
  }
  return 0;
}

/*
 * Verify that a child name still refers to the directory held by child_fd.
 * AT_SYMLINK_NOFOLLOW rejects a name swapped to a symlink after openat().
 */
static int
ftp_acl_path_verify_child_directory(int parent_fd, const char *leaf,
                                    int child_fd)
{
  struct stat opened, current;
  if (parent_fd < 0 || child_fd < 0 || !leaf || !*leaf ||
      strchr(leaf, '/') || strcmp(leaf, ".") == 0 ||
      strcmp(leaf, "..") == 0) {
    errno = EINVAL;
    return -1;
  }
  if (ftp_acl_path_is_internal_metadata(leaf)) {
    errno = EACCES;
    return -1;
  }
  if (fstat(child_fd, &opened) != 0 ||
      fstatat(parent_fd, leaf, &current, AT_SYMLINK_NOFOLLOW) != 0)
    return -1;
  if (!S_ISDIR(opened.st_mode) || !S_ISDIR(current.st_mode) ||
      opened.st_dev != current.st_dev || opened.st_ino != current.st_ino) {
    errno = ESTALE;
    return -1;
  }
  return 0;
}

/*
 * Open a direct child directory relative to its already-pinned parent.
 * O_NOFOLLOW prevents recursive LIST from descending through a substituted
 * symlink; the post-open inode comparison rejects a swapped directory entry.
 */
static int
ftp_acl_path_open_child_directory(int parent_fd, const char *leaf)
{
  int child_fd, saved_errno;
  if (parent_fd < 0 || !leaf || !*leaf || strchr(leaf, '/') ||
      strcmp(leaf, ".") == 0 || strcmp(leaf, "..") == 0) {
    errno = EINVAL;
    return -1;
  }
  if (ftp_acl_path_is_internal_metadata(leaf)) {
    errno = EACCES;
    return -1;
  }
  child_fd = openat(parent_fd, leaf,
                    O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
  if (child_fd < 0)
    return -1;
  if (ftp_acl_path_verify_child_directory(parent_fd, leaf, child_fd) != 0) {
    saved_errno = errno;
    close(child_fd);
    errno = saved_errno;
    return -1;
  }
  return child_fd;
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
  /*
   * A hard link inside an authorized share can alias an inode whose other
   * pathname is outside that share. Do not expose or modify multiply-linked
   * files through the path-based share ACL.
   */
  if (expected.st_nlink != 1) {
    errno = EACCES;
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
  if (opened.st_nlink != 1) {
    close(file_fd);
    errno = EACCES;
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
  int saved_errno;
  if (!path || !*path || !resolved_parent || resolved_size < PATH_MAX ||
      !leaf || leaf_size == 0 || !cleanup_allowed ||
      ftp_acl_path_is_internal_metadata(path) || strlen(path) >= PATH_MAX)
    return -1;

  /*
   * Path preflight never authorizes rollback: the leaf may be created or
   * replaced between this check and openat(). Only a successful O_EXCL
   * creation on the pinned parent can make a failed upload cleanup-eligible.
   */
  *cleanup_allowed = 0;
  if (realpath(path, target) != NULL) {
    if (ftp_acl_path_is_internal_metadata(target) ||
        ftp_acl_path_split_entry(target, parent, sizeof(parent),
                                 leaf, leaf_size) != 0 ||
        realpath(parent, resolved_parent) == NULL)
      return -1;
    return 0;
  }

  saved_errno = errno;
  if (saved_errno != ENOENT || lstat(path, &st) == 0 || errno != ENOENT)
    return -1;
  if (ftp_acl_path_split_entry(path, parent, sizeof(parent),
                               leaf, leaf_size) != 0 ||
      realpath(parent, resolved_parent) == NULL)
    return -1;
  return 0;
}

/*
 * Open an upload target while recording whether this session actually
 * created the leaf. Existing STOR targets are opened without O_CREAT, so a
 * failed upload can never mistake a pre-existing file for a rollback target.
 */
static int
ftp_acl_path_open_upload_entry(int dir_fd, const char *leaf, int exclusive,
                               int append, mode_t mode, int *cleanup_allowed)
{
  struct stat opened;
  int file_fd;
  int flags;
  int saved_errno;

  if (dir_fd < 0 || !leaf || !*leaf || strchr(leaf, '/') ||
      strcmp(leaf, ".") == 0 || strcmp(leaf, "..") == 0 ||
      ftp_acl_path_is_internal_metadata(leaf) || !cleanup_allowed) {
    errno = EINVAL;
    return -1;
  }

  *cleanup_allowed = 0;
  flags = O_WRONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC;
  if (append)
    flags |= O_APPEND;
  if (exclusive) {
    file_fd = openat(dir_fd, leaf,
                     flags | O_CREAT | O_EXCL, mode);
    if (file_fd >= 0)
      *cleanup_allowed = 1;
    return file_fd;
  }

  /*
   * Atomically distinguish a new leaf from an existing STOR target. If
   * another entry already exists, reopen it without O_CREAT and never mark
   * it for cleanup.
   */
  file_fd = openat(dir_fd, leaf, flags | O_CREAT | O_EXCL, mode);
  if (file_fd >= 0) {
    *cleanup_allowed = 1;
    return file_fd;
  }
  if (errno != EEXIST)
    return -1;

  file_fd = openat(dir_fd, leaf, flags, mode);
  if (file_fd < 0)
    return -1;
  if (fstat(file_fd, &opened) != 0) {
    saved_errno = errno;
    close(file_fd);
    errno = saved_errno;
    return -1;
  }
  if (!S_ISREG(opened.st_mode)) {
    close(file_fd);
    errno = EINVAL;
    return -1;
  }
  if (opened.st_nlink != 1) {
    close(file_fd);
    errno = EACCES;
    return -1;
  }
  return file_fd;
}

/*
 * Linux has no unlink-by-file-descriptor or conditional unlinkat operation.
 * Move the leaf atomically into an effective-UID-owned 0700 directory first;
 * the quarantined name cannot be replaced through the public upload directory
 * while its inode is checked.
 */
static int
ftp_acl_path_cleanup_directory_name(char *name, size_t name_size)
{
  int written;
  if (!name || name_size == 0) {
    errno = EINVAL;
    return -1;
  }
  written = snprintf(name, name_size, ".__vsftpd_cleanup_%lu",
                     (unsigned long)geteuid());
  if (written < 0 || (size_t)written >= name_size) {
    errno = ENAMETOOLONG;
    return -1;
  }
  return 0;
}

static int
ftp_acl_path_open_cleanup_directory(int dir_fd, const char *name)
{
  struct stat st;
  int cleanup_fd;
  int created = 0;
  if (dir_fd < 0 || !name ||
      !ftp_acl_path_is_internal_metadata(name)) {
    errno = EINVAL;
    return -1;
  }
  if (mkdirat(dir_fd, name, 0700) == 0)
    created = 1;
  else if (errno != EEXIST)
    return -1;

  cleanup_fd = openat(dir_fd, name,
                      O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
  if (cleanup_fd < 0)
    return -1;
  if (fstat(cleanup_fd, &st) != 0) {
    int saved_errno = errno;
    close(cleanup_fd);
    errno = saved_errno;
    return -1;
  }
  if (!S_ISDIR(st.st_mode) || st.st_uid != geteuid()) {
    close(cleanup_fd);
    errno = EPERM;
    return -1;
  }
  if (created) {
    if (fchmod(cleanup_fd, 0700) != 0) {
      int saved_errno = errno;
      close(cleanup_fd);
      errno = saved_errno;
      return -1;
    }
  } else if ((st.st_mode & 0777) != 0700) {
    close(cleanup_fd);
    errno = EPERM;
    return -1;
  }
  if (fstat(cleanup_fd, &st) != 0) {
    int saved_errno = errno;
    close(cleanup_fd);
    errno = saved_errno;
    return -1;
  }
  if (!S_ISDIR(st.st_mode) || st.st_uid != geteuid() ||
      (st.st_mode & 0777) != 0700) {
    close(cleanup_fd);
    errno = EPERM;
    return -1;
  }
  return cleanup_fd;
}

static int
ftp_acl_path_rename_noreplace(int old_dir_fd, const char *old_name,
                              int new_dir_fd, const char *new_name)
{
#if defined(SYS_renameat2)
  return (int)syscall(SYS_renameat2, old_dir_fd, old_name,
                      new_dir_fd, new_name, RENAME_NOREPLACE);
#elif defined(__NR_renameat2)
  return (int)syscall(__NR_renameat2, old_dir_fd, old_name,
                      new_dir_fd, new_name, RENAME_NOREPLACE);
#else
  (void)old_dir_fd;
  (void)old_name;
  (void)new_dir_fd;
  (void)new_name;
  errno = ENOSYS;
  return -1;
#endif
}

static int
ftp_acl_path_rename_entry(int source_dir_fd, const char *source_leaf,
                          int destination_dir_fd,
                          const char *destination_leaf)
{
  static unsigned int sequence;
  struct stat quarantined;
  char cleanup_dir[96], quarantine_leaf[96];
  unsigned int attempt;
  int cleanup_fd, moved = 0, written, saved_errno, stat_status;

  if (source_dir_fd < 0 || destination_dir_fd < 0 ||
      !source_leaf || !*source_leaf || strchr(source_leaf, '/') ||
      strcmp(source_leaf, ".") == 0 || strcmp(source_leaf, "..") == 0 ||
      !destination_leaf || !*destination_leaf ||
      strchr(destination_leaf, '/') ||
      strcmp(destination_leaf, ".") == 0 ||
      strcmp(destination_leaf, "..") == 0) {
    errno = EINVAL;
    return -1;
  }
  if (ftp_acl_path_is_internal_metadata(source_leaf) ||
      ftp_acl_path_is_internal_metadata(destination_leaf)) {
    errno = EACCES;
    return -1;
  }
  if (ftp_acl_path_cleanup_directory_name(cleanup_dir,
                                         sizeof(cleanup_dir)) != 0)
    return -1;
  cleanup_fd = ftp_acl_path_open_cleanup_directory(source_dir_fd,
                                                    cleanup_dir);
  if (cleanup_fd < 0)
    return -1;

  /*
   * Checking the public leaf and then renaming it is racy: another writer
   * could replace it with a hard link between fstatat() and renameat().
   * Move the exact current entry into the private directory first, then
   * validate the moved inode before publishing it to the destination.
   */
  for (attempt = 0; attempt < 32U; ++attempt) {
    ++sequence;
    written = snprintf(quarantine_leaf, sizeof(quarantine_leaf),
                       "rename-%ld-%u", (long)getpid(), sequence);
    if (written < 0 || (size_t)written >= sizeof(quarantine_leaf)) {
      close(cleanup_fd);
      errno = ENAMETOOLONG;
      return -1;
    }
    if (ftp_acl_path_rename_noreplace(source_dir_fd, source_leaf,
                                      cleanup_fd, quarantine_leaf) == 0) {
      moved = 1;
      break;
    }
    if (errno != EEXIST) {
      saved_errno = errno;
      close(cleanup_fd);
      errno = saved_errno;
      return -1;
    }
  }
  if (!moved) {
    close(cleanup_fd);
    errno = EEXIST;
    return -1;
  }

  stat_status = fstatat(cleanup_fd, quarantine_leaf, &quarantined,
                        AT_SYMLINK_NOFOLLOW);
  if (stat_status != 0) {
    saved_errno = errno;
  } else if (!S_ISDIR(quarantined.st_mode) && quarantined.st_nlink != 1) {
    /* Directory link counts naturally exceed one; reject aliased leaves. */
    saved_errno = EACCES;
  } else if (renameat(cleanup_fd, quarantine_leaf, destination_dir_fd,
                      destination_leaf) == 0) {
    close(cleanup_fd);
    return 0;
  } else {
    saved_errno = errno;
  }

  /*
   * Validation or publication failed. Restore only if the original public
   * name is still free; never overwrite a replacement created by another
   * writer. If restoration loses that race, retain the item in quarantine.
   */
  if (ftp_acl_path_rename_noreplace(cleanup_fd, quarantine_leaf,
                                    source_dir_fd, source_leaf) != 0) {
    int restore_errno = errno;
    close(cleanup_fd);
    errno = restore_errno;
    return -1;
  }
  close(cleanup_fd);
  errno = saved_errno;
  return -1;
}

static int
ftp_acl_path_unlink_opened_entry(int dir_fd, const char *leaf, int file_fd)
{
  static unsigned int sequence;
  struct stat opened, quarantined;
  char cleanup_dir[96], quarantine_leaf[96];
  unsigned int attempt;
  int cleanup_fd, moved = 0, written, saved_errno, stat_status;

  if (dir_fd < 0 || !leaf || !*leaf || strchr(leaf, '/') ||
      strcmp(leaf, ".") == 0 || strcmp(leaf, "..") == 0 ||
      ftp_acl_path_is_internal_metadata(leaf) || file_fd < 0) {
    errno = EINVAL;
    return -1;
  }
  if (fstat(file_fd, &opened) != 0)
    return -1;
  if (!S_ISREG(opened.st_mode)) {
    errno = EINVAL;
    return -1;
  }
  /* Rollback must not unlink a pathname that has gained an outside alias. */
  if (opened.st_nlink != 1) {
    errno = EACCES;
    return -1;
  }
  if (ftp_acl_path_cleanup_directory_name(cleanup_dir,
                                         sizeof(cleanup_dir)) != 0)
    return -1;
  cleanup_fd = ftp_acl_path_open_cleanup_directory(dir_fd, cleanup_dir);
  if (cleanup_fd < 0)
    return -1;

  /* PID plus sequence avoids collisions between concurrent vsftpd children. */
  for (attempt = 0; attempt < 32U; ++attempt) {
    ++sequence;
    written = snprintf(quarantine_leaf, sizeof(quarantine_leaf),
                       "upload-%ld-%u", (long)getpid(), sequence);
    if (written < 0 || (size_t)written >= sizeof(quarantine_leaf)) {
      close(cleanup_fd);
      errno = ENAMETOOLONG;
      return -1;
    }
    if (ftp_acl_path_rename_noreplace(dir_fd, leaf, cleanup_fd,
                                      quarantine_leaf) == 0) {
      moved = 1;
      break;
    }
    if (errno != EEXIST) {
      saved_errno = errno;
      close(cleanup_fd);
      errno = saved_errno;
      return -1;
    }
  }
  if (!moved) {
    close(cleanup_fd);
    errno = EEXIST;
    return -1;
  }

  stat_status = fstatat(cleanup_fd, quarantine_leaf, &quarantined,
                        AT_SYMLINK_NOFOLLOW);
  if (stat_status == 0 && S_ISREG(quarantined.st_mode) &&
      opened.st_dev == quarantined.st_dev &&
      opened.st_ino == quarantined.st_ino) {
    if (quarantined.st_nlink != 1) {
      saved_errno = EACCES;
    } else if (unlinkat(cleanup_fd, quarantine_leaf, 0) == 0) {
      close(cleanup_fd);
      return 0;
    } else {
      saved_errno = errno;
    }
  } else if (stat_status == 0) {
    saved_errno = ESTALE;
  } else {
    saved_errno = errno;
  }

  /*
   * Never discard a mismatched or unverified entry. Restore only if the
   * public leaf is still absent; if another writer won the name, retain the
   * quarantined object for recovery and fail the cleanup operation.
   */
  if (ftp_acl_path_rename_noreplace(cleanup_fd, quarantine_leaf,
                                    dir_fd, leaf) != 0) {
    int restore_errno = errno;
    close(cleanup_fd);
    errno = restore_errno;
    return -1;
  }
  close(cleanup_fd);
  errno = saved_errno;
  return -1;
}
static int
ftp_acl_path_resolve_directory(const char *path, char *resolved,
                               size_t resolved_size)
{
  if (!path || !*path || !resolved || resolved_size < PATH_MAX)
    return -1;
  if (realpath(path, resolved) == NULL)
    return -1;
  if (ftp_acl_path_is_internal_metadata(resolved)) {
    errno = EACCES;
    return -1;
  }
  return 0;
}



/*
 * Build a child pathname from an already-canonical listing directory. The
 * base is resolved once per directory, not once per entry, to avoid adding
 * realpath()/stat overhead to large listings.
 */
static int
ftp_acl_path_build_child_path(const char *resolved_parent, const char *leaf,
                              char *child_path, size_t child_size)
{
  int length;
  if (!resolved_parent || resolved_parent[0] != '/' || !leaf || !*leaf ||
      strchr(leaf, '/') || strcmp(leaf, ".") == 0 ||
      strcmp(leaf, "..") == 0 || !child_path || child_size == 0) {
    errno = EINVAL;
    return -1;
  }
  if (strcmp(resolved_parent, "/") == 0)
    length = snprintf(child_path, child_size, "/%s", leaf);
  else
    length = snprintf(child_path, child_size, "%s/%s", resolved_parent, leaf);
  if (length < 0 || (size_t) length >= child_size) {
    errno = ENAMETOOLONG;
    return -1;
  }
  return 0;
}

/*
 * Create a directory entry under an already-authorized, pinned parent.
 * The final component is never re-resolved after the WRITE ACL check.
 */
/*
 * Open a bounded regular metadata file relative to an already-pinned
 * directory. O_NOFOLLOW protects the final component; O_NONBLOCK prevents a
 * swapped FIFO/device node from hanging the privileged FTP process.
 */
static int
ftp_acl_path_open_bounded_regular_file_at(int directory_fd, const char *leaf,
                                          off_t max_size,
                                          struct stat *p_stat)
{
  struct stat opened;
  int fd, saved_errno;

  if (directory_fd < 0 || !leaf || !*leaf || strchr(leaf, '/') ||
      strcmp(leaf, ".") == 0 || strcmp(leaf, "..") == 0 ||
      max_size < 1 || !p_stat) {
    errno = EINVAL;
    return -1;
  }
  fd = openat(directory_fd, leaf,
              O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK);
  if (fd < 0)
    return -1;
  if (fstat(fd, &opened) != 0)
    goto open_error;
  if (!S_ISREG(opened.st_mode) || opened.st_size < 1 ||
      opened.st_size > max_size) {
    errno = EINVAL;
    goto open_error;
  }
  *p_stat = opened;
  return fd;

open_error:
  saved_errno = errno;
  close(fd);
  errno = saved_errno;
  return -1;
}

/*
 * Compatibility wrapper for host regressions and legacy callers. Pin the
 * complete parent path and verify it still names that directory after open.
 */
static int ftp_acl_path_split_entry(const char *path, char *parent,
                                    size_t parent_size, char *leaf,
                                    size_t leaf_size);
static int ftp_acl_path_open_verified_directory(const char *path);
static int ftp_acl_path_verify_directory_identity(const char *path,
                                                   int dir_fd);

static int
ftp_acl_path_open_bounded_regular_file(const char *path, off_t max_size,
                                       struct stat *p_stat)
{
  char parent[PATH_MAX], leaf[PATH_MAX];
  int dir_fd, fd, saved_errno;

  if (!path || path[0] != '/' || max_size < 1 || !p_stat) {
    errno = EINVAL;
    return -1;
  }
  if (ftp_acl_path_split_entry(path, parent, sizeof(parent),
                               leaf, sizeof(leaf)) != 0) {
    errno = EACCES;
    return -1;
  }
  dir_fd = ftp_acl_path_open_verified_directory(parent);
  if (dir_fd < 0)
    return -1;

  fd = ftp_acl_path_open_bounded_regular_file_at(
      dir_fd, leaf, max_size, p_stat);
  if (fd < 0) {
    saved_errno = errno ? errno : EIO;
    close(dir_fd);
    errno = saved_errno;
    return -1;
  }
  if (ftp_acl_path_verify_directory_identity(parent, dir_fd) != 0) {
    saved_errno = errno ? errno : ESTALE;
    close(fd);
    close(dir_fd);
    errno = saved_errno;
    return -1;
  }
  if (close(dir_fd) != 0) {
    saved_errno = errno ? errno : EIO;
    close(fd);
    errno = saved_errno;
    return -1;
  }
  return fd;
}

static int
ftp_acl_path_make_directory_at(int dir_fd, const char *leaf,
                               unsigned int mode)
{
  if (dir_fd < 0 || !leaf || !*leaf || strchr(leaf, '/') ||
      strcmp(leaf, ".") == 0 || strcmp(leaf, "..") == 0) {
    errno = EINVAL;
    return -1;
  }
  if (ftp_acl_path_is_internal_metadata(leaf)) {
    errno = EACCES;
    return -1;
  }
  return mkdirat(dir_fd, leaf, (mode_t)mode);
}

/*
 * Remove a leaf from an already-authorized directory. unlinkat() with no
 * flags preserves DELE's unlink-the-symlink behavior; AT_REMOVEDIR preserves
 * RMD's non-following directory semantics.
 */
static int
ftp_acl_path_remove_entry(int dir_fd, const char *leaf, int remove_directory)
{
  if (dir_fd < 0 || !leaf || !*leaf || strchr(leaf, '/') ||
      strcmp(leaf, ".") == 0 || strcmp(leaf, "..") == 0) {
    errno = EINVAL;
    return -1;
  }
  if (ftp_acl_path_is_internal_metadata(leaf)) {
    errno = EACCES;
    return -1;
  }
  return unlinkat(dir_fd, leaf, remove_directory ? AT_REMOVEDIR : 0);
}


/*
 * fchdir(O_PATH) is supported on Linux 3.5+. Older router kernels return
 * EBADF; /proc/self/fd/N still resolves the already-pinned directory inode.
 */
static int
ftp_acl_path_chdir_procfd(int directory_fd)
{
  struct stat st;
  char fd_path[64];
  int length;

  if (directory_fd < 0)
  {
    errno = EBADF;
    return -1;
  }
  if (fstat(directory_fd, &st) != 0)
    return -1;
  if (!S_ISDIR(st.st_mode))
  {
    errno = ENOTDIR;
    return -1;
  }
  length = snprintf(fd_path, sizeof(fd_path), "/proc/self/fd/%d",
                    directory_fd);
  if (length < 0 || (size_t) length >= sizeof(fd_path))
  {
    errno = ENAMETOOLONG;
    return -1;
  }
  return chdir(fd_path);
}

static int
ftp_acl_path_chdir(int directory_fd)
{
  if (fchdir(directory_fd) == 0)
    return 0;
  if (errno != EBADF)
    return -1;
  return ftp_acl_path_chdir_procfd(directory_fd);
}

#endif /* FTP_ACL_PATH_POLICY_H */

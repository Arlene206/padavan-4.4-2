#ifndef FTP_ACL_PATH_POLICY_H
#define FTP_ACL_PATH_POLICY_H
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
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

static int
ftp_acl_path_resolve_existing_parent(const char *path, char *resolved_parent,
                                     size_t resolved_size)
{
  char target[PATH_MAX], parent[PATH_MAX];
  if (!path || !*path || !resolved_parent || resolved_size < PATH_MAX)
    return -1;
  if (realpath(path, target) == NULL)
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

static int
ftp_acl_path_resolve_directory(const char *path, char *resolved,
                               size_t resolved_size)
{
  if (!path || !*path || !resolved || resolved_size < PATH_MAX)
    return -1;
  return realpath(path, resolved) == NULL ? -1 : 0;
}

#endif /* FTP_ACL_PATH_POLICY_H */

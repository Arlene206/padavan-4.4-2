#ifndef FTP_ACL_PATH_POLICY_H
#define FTP_ACL_PATH_POLICY_H
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
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
#endif /* FTP_ACL_PATH_POLICY_H */

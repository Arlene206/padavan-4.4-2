#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include "ftp_acl_path_policy.h"

static void
expect_parent(const char *path, const char *expected)
{
  char parent[PATH_MAX] = "";
  if (ftp_acl_path_parent(path, parent, sizeof(parent)) != 0 ||
      strcmp(parent, expected) != 0) {
    fprintf(stderr, "parent mismatch for '%s': '%s' != '%s'\n",
            path ? path : "(null)", parent, expected);
    exit(1);
  }
}
static void
checked_path(char *out, size_t size, const char *base, const char *name)
{
  int n = snprintf(out, size, "%s/%s", base, name);
  if (n < 0 || (size_t)n >= size) {
    fprintf(stderr, "fixture path overflow\n");
    exit(1);
  }
}
int
main(void)
{
  char template[] = "/tmp/ftp-acl-policy-XXXXXX";
  char cwd[PATH_MAX], root[PATH_MAX], a[PATH_MAX], b[PATH_MAX];
  char alias[PATH_MAX], path[PATH_MAX], parent[PATH_MAX], resolved[PATH_MAX];
  char tiny[2];
  char *created;

  expect_parent("file", ".");
  expect_parent("share/file", "share");
  expect_parent("../share/file", "../share");
  expect_parent("/share/file", "/share");
  expect_parent("/share/dir/", "/share");
  expect_parent("/", "/");
  expect_parent("////", "/");
  if (ftp_acl_path_parent("", parent, sizeof(parent)) == 0 ||
      ftp_acl_path_parent(NULL, parent, sizeof(parent)) == 0 ||
      ftp_acl_path_parent("/long/path", tiny, sizeof(tiny)) == 0) {
    fprintf(stderr, "invalid path/buffer accepted\n"); return 1;
  }
  if (!getcwd(cwd, sizeof(cwd)) || !(created = mkdtemp(template))) {
    perror("prepare fixture"); return 1;
  }
  if (snprintf(root, sizeof(root), "%s", created) < 0 ||
      strlen(created) >= sizeof(root)) {
    fprintf(stderr, "fixture path overflow\n"); return 1;
  }
  checked_path(a, sizeof(a), root, "shareA");
  checked_path(b, sizeof(b), root, "shareB");
  checked_path(alias, sizeof(alias), root, "alias");
  if (mkdir(a, 0700) || mkdir(b, 0700) || symlink(b, alias)) {
    perror("create fixture"); return 1;
  }
  checked_path(path, sizeof(path), a, "file");
  if (ftp_acl_path_resolve_parent(path, parent, sizeof(parent),
                                  resolved, sizeof(resolved)) ||
      strcmp(resolved, a)) {
    fprintf(stderr, "absolute parent resolution failed\n"); return 1;
  }
  checked_path(path, sizeof(path), alias, "file");
  if (ftp_acl_path_resolve_parent(path, parent, sizeof(parent),
                                  resolved, sizeof(resolved)) ||
      strcmp(resolved, b)) {
    fprintf(stderr, "symlinked parent was not canonicalized\n"); return 1;
  }
  if (chdir(a) || ftp_acl_path_resolve_parent("../shareB/file", parent,
        sizeof(parent), resolved, sizeof(resolved)) || strcmp(resolved, b)) {
    fprintf(stderr, "relative traversal parent was not canonicalized\n"); return 1;
  }
  if (chdir(cwd) || ftp_acl_path_resolve_parent("/nonexistent-parent/file",
        parent, sizeof(parent), resolved, sizeof(resolved)) == 0) {
    fprintf(stderr, "cwd restore or missing-parent rejection failed\n"); return 1;
  }
  if (unlink(alias) || rmdir(a) || rmdir(b) || rmdir(root)) {
    perror("remove fixture"); return 1;
  }
  puts("vsftpd ACL path parent policy: PASS");
  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include "ftp_acl_path_policy.h"


static void
create_fixture_file(const char *path)
{
  FILE *fp = fopen(path, "w");
  if (!fp) {
    perror("create fixture file");
    exit(1);
  }
  if (fputs("fixture\n", fp) < 0) {
    fclose(fp);
    perror("write fixture file");
    exit(1);
  }
  if (fclose(fp) != 0) {
    perror("close fixture file");
    exit(1);
  }
}

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
  char file_a[PATH_MAX], file_b[PATH_MAX], link_file[PATH_MAX];
  char broken_link[PATH_MAX], dir_b[PATH_MAX], dir_alias[PATH_MAX];
  char new_file[PATH_MAX];
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

  checked_path(file_a, sizeof(file_a), a, "owned.txt");
  checked_path(file_b, sizeof(file_b), b, "protected.txt");
  checked_path(link_file, sizeof(link_file), a, "cross-share-link");
  checked_path(broken_link, sizeof(broken_link), a, "dangling-link");
  checked_path(dir_b, sizeof(dir_b), b, "subdir");
  checked_path(dir_alias, sizeof(dir_alias), a, "directory-link");
  create_fixture_file(file_a);
  create_fixture_file(file_b);
  if (mkdir(dir_b, 0700) || symlink(file_b, link_file) ||
      symlink("/no-such-ftp-acl-target", broken_link) ||
      symlink(dir_b, dir_alias)) {
    perror("create symlink fixtures"); return 1;
  }
  if (ftp_acl_path_resolve_existing_parent(file_a, resolved, sizeof(resolved)) ||
      strcmp(resolved, a)) {
    fprintf(stderr, "existing file parent resolution failed\n"); return 1;
  }
  if (ftp_acl_path_resolve_existing_parent(link_file, resolved, sizeof(resolved)) ||
      strcmp(resolved, b)) {
    fprintf(stderr, "read through cross-share symlink was not canonicalized\n"); return 1;
  }
  if (ftp_acl_path_resolve_write_parent(link_file, resolved, sizeof(resolved)) ||
      strcmp(resolved, b)) {
    fprintf(stderr, "write through cross-share symlink was not canonicalized\n"); return 1;
  }
  if (ftp_acl_path_resolve_write_parent(broken_link, resolved, sizeof(resolved)) == 0) {
    fprintf(stderr, "dangling final symlink accepted for upload\n"); return 1;
  }
  checked_path(new_file, sizeof(new_file), a, "new-upload.txt");
  if (ftp_acl_path_resolve_write_parent(new_file, resolved, sizeof(resolved)) ||
      strcmp(resolved, a)) {
    fprintf(stderr, "new upload parent resolution failed\n"); return 1;
  }
  checked_path(new_file, sizeof(new_file), dir_alias, "new-through-dir-link.txt");
  if (ftp_acl_path_resolve_write_parent(new_file, resolved, sizeof(resolved)) ||
      strcmp(resolved, dir_b)) {
    fprintf(stderr, "upload through symlinked parent was not canonicalized\n"); return 1;
  }
  if (ftp_acl_path_resolve_directory(dir_alias, resolved, sizeof(resolved)) ||
      strcmp(resolved, dir_b)) {
    fprintf(stderr, "directory symlink target was not canonicalized\n"); return 1;
  }
  if (unlink(dir_alias) || unlink(broken_link) || unlink(link_file) ||
      unlink(file_a) || unlink(file_b) || rmdir(dir_b) ||
      unlink(alias) || rmdir(a) || rmdir(b) || rmdir(root)) {
    perror("remove fixture"); return 1;
  }
  puts("vsftpd ACL path parent policy: PASS");
  return 0;
}

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
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
  char metadata_file[PATH_MAX], metadata_alias[PATH_MAX];
  char new_file[PATH_MAX], leaf[PATH_MAX];
  char held_a[PATH_MAX], race_link[PATH_MAX];
  char rollback_path[PATH_MAX], rollback_saved[PATH_MAX];
  char tiny[2];
  int cleanup_allowed, dir_fd, race_fd, rollback_fd, replacement_fd;
  struct stat st;
  char *created;

  if (!ftp_acl_path_is_internal_metadata("/mnt/.__folder_list.txt") ||
      !ftp_acl_path_is_internal_metadata(".__alice_var.txt") ||
      ftp_acl_path_is_internal_metadata("/mnt/share/.__notes.txt") ||
      ftp_acl_path_is_internal_metadata("/mnt/share/.visible-file") ||
      ftp_acl_path_is_internal_metadata("/mnt/.__alice_var.tx")) {
    fprintf(stderr, "reserved FTP metadata classification failed\n");
    return 1;
  }
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
  checked_path(metadata_file, sizeof(metadata_file), root, ".__alice_var.txt");
  checked_path(metadata_alias, sizeof(metadata_alias), a, "metadata-alias");
  create_fixture_file(file_a);
  create_fixture_file(file_b);
  create_fixture_file(metadata_file);
  if (mkdir(dir_b, 0700) || symlink(file_b, link_file) ||
      symlink("/no-such-ftp-acl-target", broken_link) ||
      symlink(dir_b, dir_alias) || symlink(metadata_file, metadata_alias)) {
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
  if (ftp_acl_path_resolve_write_entry(link_file, resolved, sizeof(resolved),
                                       leaf, sizeof(leaf), &cleanup_allowed) ||
      strcmp(resolved, b) || strcmp(leaf, "protected.txt") ||
      cleanup_allowed) {
    fprintf(stderr, "canonical upload symlink target resolution failed\n"); return 1;
  }
  if (ftp_acl_path_resolve_write_parent(broken_link, resolved, sizeof(resolved)) == 0 ||
      ftp_acl_path_resolve_write_entry(broken_link, resolved, sizeof(resolved),
                                       leaf, sizeof(leaf), &cleanup_allowed) == 0) {
    fprintf(stderr, "dangling final symlink accepted for upload\n"); return 1;
  }
  if (ftp_acl_path_resolve_existing_parent(metadata_file, resolved, sizeof(resolved)) == 0 ||
      ftp_acl_path_resolve_existing_parent(metadata_alias, resolved, sizeof(resolved)) == 0 ||
      ftp_acl_path_resolve_write_parent(metadata_alias, resolved, sizeof(resolved)) == 0) {
    fprintf(stderr, "reserved metadata file or symlink alias was accepted\n"); return 1;
  }
  checked_path(new_file, sizeof(new_file), a, "new-upload.txt");
  if (ftp_acl_path_resolve_write_parent(new_file, resolved, sizeof(resolved)) ||
      strcmp(resolved, a) ||
      ftp_acl_path_resolve_write_entry(new_file, resolved, sizeof(resolved),
                                       leaf, sizeof(leaf), &cleanup_allowed) ||
      strcmp(resolved, a) || strcmp(leaf, "new-upload.txt") ||
      !cleanup_allowed) {
    fprintf(stderr, "new upload parent/leaf resolution failed\n"); return 1;
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

  /*
   * A directory descriptor must remain attached to the authorized directory
   * when its pathname is replaced by a symlink to another share.
   */
  checked_path(held_a, sizeof(held_a), root, "shareA-held");
  dir_fd = ftp_acl_path_open_directory(a);
  if (dir_fd < 0 || rename(a, held_a) || symlink(b, a)) {
    perror("pin authorized upload directory");
    return 1;
  }
  race_fd = openat(dir_fd, "dirfd-created.txt",
                   O_CREAT | O_EXCL | O_WRONLY | O_NOFOLLOW | O_CLOEXEC,
                   0600);
  if (race_fd < 0 || write(race_fd, "pinned", 6) != 6 || close(race_fd) != 0) {
    perror("create entry through pinned directory");
    return 1;
  }
  checked_path(path, sizeof(path), held_a, "dirfd-created.txt");
  if (stat(path, &st) != 0) {
    fprintf(stderr, "directory descriptor did not stay on original share\n");
    return 1;
  }
  checked_path(path, sizeof(path), a, "dirfd-created.txt");
  if (lstat(path, &st) == 0) {
    fprintf(stderr, "directory descriptor followed replaced share symlink\n");
    return 1;
  }

  checked_path(race_link, sizeof(race_link), held_a, "race-link");
  if (symlink(file_b, race_link) != 0) {
    perror("create final symlink race fixture");
    return 1;
  }
  errno = 0;
  race_fd = openat(dir_fd, "race-link",
                   O_WRONLY | O_NOFOLLOW | O_CLOEXEC);
  if (race_fd >= 0 || errno != ELOOP) {
    fprintf(stderr, "O_NOFOLLOW accepted a substituted final symlink\n");
    return 1;
  }

  /*
   * Cleanup must not unlink a different inode already installed at the
   * upload leaf. This covers replacement before the identity check.
   */
  checked_path(rollback_path, sizeof(rollback_path), held_a, "rollback.txt");
  checked_path(rollback_saved, sizeof(rollback_saved), held_a, "rollback.saved");
  rollback_fd = openat(dir_fd, "rollback.txt",
                       O_CREAT | O_EXCL | O_WRONLY | O_NOFOLLOW | O_CLOEXEC,
                       0600);
  if (rollback_fd < 0 || write(rollback_fd, "old", 3) != 3 ||
      renameat(dir_fd, "rollback.txt", dir_fd, "rollback.saved") != 0) {
    perror("prepare upload cleanup replacement");
    return 1;
  }
  replacement_fd = openat(dir_fd, "rollback.txt",
                           O_CREAT | O_EXCL | O_WRONLY | O_NOFOLLOW | O_CLOEXEC,
                           0600);
  if (replacement_fd < 0 || write(replacement_fd, "new", 3) != 3) {
    perror("create replacement upload entry");
    return 1;
  }
  if (ftp_acl_path_unlink_opened_entry(dir_fd, "rollback.txt", rollback_fd) == 0) {
    fprintf(stderr, "cleanup removed a replacement upload entry\n");
    return 1;
  }
  if (stat(rollback_path, &st) != 0 || stat(rollback_saved, &st) != 0) {
    fprintf(stderr, "cleanup replacement fixtures disappeared\n");
    return 1;
  }
  close(rollback_fd);
  close(replacement_fd);
  if (unlinkat(dir_fd, "rollback.txt", 0) != 0 ||
      unlinkat(dir_fd, "rollback.saved", 0) != 0 ||
      unlinkat(dir_fd, "dirfd-created.txt", 0) != 0 ||
      unlinkat(dir_fd, "race-link", 0) != 0 ||
      close(dir_fd) != 0 || unlink(a) != 0 || rename(held_a, a) != 0) {
    perror("restore descriptor race fixtures");
    return 1;
  }

  if (unlink(metadata_alias) || unlink(metadata_file) ||
      unlink(dir_alias) || unlink(broken_link) || unlink(link_file) ||
      unlink(file_a) || unlink(file_b) || rmdir(dir_b) ||
      unlink(alias) || rmdir(a) || rmdir(b) || rmdir(root)) {
    perror("remove fixture"); return 1;
  }
  puts("vsftpd ACL path parent policy: PASS");
  return 0;
}

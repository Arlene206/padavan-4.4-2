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
#include "ftp_listing_policy.h"


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
  int existing_dir_fd, existing_file_fd, remove_parent_fd;
  int child_parent_fd, child_dir_fd;
  int rename_source_fd, rename_destination_fd, metadata_fd;
  char existing_leaf[PATH_MAX], readback[16];
  char link_target[PATH_MAX];
  char remove_link[PATH_MAX], remove_dir_link[PATH_MAX], empty_dir[PATH_MAX];
  char rename_source[PATH_MAX], rename_destination[PATH_MAX];
  char rename_link[PATH_MAX], rename_link_destination[PATH_MAX];
  char rename_dir[PATH_MAX], rename_dir_link[PATH_MAX];
  char rename_parent_a[PATH_MAX], rename_parent_b[PATH_MAX];
  char fifo_file[PATH_MAX];
  ssize_t read_length, link_target_length;
  struct stat st, canonical_st;
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
  if (vsf_listing_entry_limit_reached(
          VSF_LISTING_MAX_ENTRIES_PER_DIR - 1, 0) ||
      !vsf_listing_entry_limit_reached(VSF_LISTING_MAX_ENTRIES_PER_DIR, 0) ||
      !vsf_listing_entry_limit_reached(0, VSF_LISTING_MAX_ENTRIES_TOTAL) ||
      vsf_listing_depth_limit_reached(VSF_LISTING_MAX_DEPTH - 1, 1) ||
      !vsf_listing_depth_limit_reached(VSF_LISTING_MAX_DEPTH, 1) ||
      vsf_listing_depth_limit_reached(VSF_LISTING_MAX_DEPTH, 0)) {
    fprintf(stderr, "FTP listing resource limits are inconsistent\n");
    return 1;
  }
  if (strcmp(VSF_LISTING_STAT_COMPLETE_TEXT, "End of status") != 0 ||
      strcmp(VSF_LISTING_STAT_INCOMPLETE_TEXT,
             "End of status (listing unavailable or incomplete)") != 0) {
    fprintf(stderr, "FTP STAT completion message policy failed\n");
    return 1;
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
  checked_path(remove_link, sizeof(remove_link), a, "remove-link");
  checked_path(remove_dir_link, sizeof(remove_dir_link), a, "remove-dir-link");
  checked_path(empty_dir, sizeof(empty_dir), a, "empty-dir");
  create_fixture_file(file_a);
  create_fixture_file(file_b);
  if (chdir(b) ||
      ftp_acl_path_build_child_path(a, "owned.txt", resolved, sizeof(resolved)) != 0 ||
      strcmp(resolved, file_a) != 0 ||
      ftp_acl_path_build_child_path(a, "../owned.txt", resolved,
                                    sizeof(resolved)) == 0 ||
      ftp_acl_path_build_child_path(a, "bad/name", resolved,
                                    sizeof(resolved)) == 0 ||
      chdir(cwd) != 0) {
    fprintf(stderr, "listing child path did not honor the canonical base or accepted traversal\n");
    return 1;
  }

  create_fixture_file(metadata_file);
  if (mkdir(dir_b, 0700) || symlink(file_b, link_file) ||
      symlink("/no-such-ftp-acl-target", broken_link) ||
      symlink(dir_b, dir_alias) || symlink(metadata_file, metadata_alias) ||
      symlink(file_b, remove_link) || symlink(dir_b, remove_dir_link) ||
      mkdir(empty_dir, 0700)) {
    perror("create symlink/removal fixtures"); return 1;
  }
  checked_path(fifo_file, sizeof(fifo_file), a, "acl-config-fifo");
  if (mkfifo(fifo_file, 0600) != 0) {
    perror("create ACL config FIFO fixture"); return 1;
  }
  metadata_fd = ftp_acl_path_open_bounded_regular_file(
      metadata_file, 16 * 1024 * 1024, &st);
  if (metadata_fd < 0 || !S_ISREG(st.st_mode) || st.st_size <= 0 ||
      close(metadata_fd) != 0) {
    fprintf(stderr, "regular ACL config file was not opened and verified\n");
    return 1;
  }
  if (ftp_acl_path_open_bounded_regular_file(
          fifo_file, 16 * 1024 * 1024, &st) >= 0) {
    fprintf(stderr, "ACL config FIFO was accepted (or blocked the reader)\n");
    return 1;
  }
  if (ftp_acl_path_open_bounded_regular_file(
          metadata_alias, 16 * 1024 * 1024, &st) >= 0) {
    fprintf(stderr, "ACL config symlink was followed\n");
    return 1;
  }
  if (ftp_acl_path_open_bounded_regular_file(metadata_file, 1, &st) >= 0) {
    fprintf(stderr, "oversized ACL config fixture was accepted\n");
    return 1;
  }
  /*
   * LIST metadata must be obtained relative to the directory descriptor:
   * lstat semantics retain symlink identity and readlinkat sees its target.
   */
  child_parent_fd = ftp_acl_path_open_verified_directory(a);
  if (child_parent_fd < 0 ||
      fstatat(child_parent_fd, "cross-share-link", &st,
              AT_SYMLINK_NOFOLLOW) != 0 || !S_ISLNK(st.st_mode)) {
    fprintf(stderr, "descriptor-relative lstat did not preserve symlink entry identity\n");
    return 1;
  }
  link_target_length = readlinkat(child_parent_fd, "cross-share-link",
                                  link_target, sizeof(link_target) - 1);
  if (link_target_length < 0 ||
      (size_t) link_target_length >= sizeof(link_target)) {
    fprintf(stderr, "descriptor-relative readlink failed\n");
    return 1;
  }
  link_target[link_target_length] = '\0';
  if (strcmp(link_target, file_b) != 0 || close(child_parent_fd) != 0) {
    fprintf(stderr, "descriptor-relative readlink returned the wrong target\n");
    return 1;
  }

  /*
   * Recursive listing must open direct child directories through the pinned
   * parent and must not follow a directory symlink substituted for a child.
   */
  child_parent_fd = ftp_acl_path_open_verified_directory(b);
  child_dir_fd = child_parent_fd < 0 ? -1 :
      ftp_acl_path_open_child_directory(child_parent_fd, "subdir");
  if (child_dir_fd < 0 || fstat(child_dir_fd, &st) != 0 ||
      stat(dir_b, &canonical_st) != 0 || !S_ISDIR(st.st_mode) ||
      st.st_dev != canonical_st.st_dev || st.st_ino != canonical_st.st_ino) {
    fprintf(stderr, "recursive child open did not pin the direct directory\n");
    return 1;
  }
  if (close(child_dir_fd) != 0 || close(child_parent_fd) != 0) {
    perror("close recursive child directory fixture");
    return 1;
  }
  child_parent_fd = ftp_acl_path_open_verified_directory(a);
  if (child_parent_fd < 0) {
    perror("open recursive symlink parent");
    return 1;
  }
  child_dir_fd = ftp_acl_path_open_child_directory(child_parent_fd,
                                                    "directory-link");
  if (child_dir_fd >= 0) {
    close(child_dir_fd);
    close(child_parent_fd);
    fprintf(stderr, "recursive child open followed a substituted directory symlink\n");
    return 1;
  }
  if (close(child_parent_fd) != 0) {
    perror("close recursive symlink parent");
    return 1;
  }

  /*
   * DELE must unlink the final symlink itself; RMD must not follow a symlink
   * to a directory. The same helper also supports removing an empty directory.
   */
  remove_parent_fd = ftp_acl_path_open_verified_directory(a);
  if (remove_parent_fd < 0 ||
      ftp_acl_path_remove_entry(remove_parent_fd, "remove-link", 0) != 0 ||
      lstat(remove_link, &st) == 0 || stat(file_b, &st) != 0) {
    fprintf(stderr, "DELE removed symlink target instead of link entry\n");
    return 1;
  }
  errno = 0;
  if (ftp_acl_path_remove_entry(remove_parent_fd, "remove-dir-link", 1) == 0 ||
      lstat(remove_dir_link, &st) != 0 || stat(dir_b, &st) != 0) {
    fprintf(stderr, "RMD followed or removed a final directory symlink\n");
    return 1;
  }
  if (ftp_acl_path_remove_entry(remove_parent_fd, "remove-dir-link", 0) != 0 ||
      lstat(remove_dir_link, &st) == 0) {
    fprintf(stderr, "DELE could not remove the directory symlink entry\n");
    return 1;
  }
  if (ftp_acl_path_remove_entry(remove_parent_fd, "empty-dir", 1) != 0 ||
      lstat(empty_dir, &st) == 0) {
    fprintf(stderr, "RMD could not remove an empty directory entry\n");
    return 1;
  }
  errno = 0;
  if (ftp_acl_path_remove_entry(remove_parent_fd, ".__alice_var.txt", 0) == 0 ||
      errno != EACCES) {
    fprintf(stderr, "descriptor-relative removal accepted reserved metadata\n");
    return 1;
  }
  if (ftp_acl_path_make_directory_at(remove_parent_fd, "mkdir-created", 0777) != 0 ||
      fstatat(remove_parent_fd, "mkdir-created", &st, AT_SYMLINK_NOFOLLOW) != 0 ||
      !S_ISDIR(st.st_mode)) {
    fprintf(stderr, "descriptor-relative MKD did not create directory\n");
    return 1;
  }
  errno = 0;
  if (ftp_acl_path_make_directory_at(remove_parent_fd, ".__alice_var.txt", 0777) == 0 ||
      errno != EACCES) {
    fprintf(stderr, "descriptor-relative MKD accepted reserved metadata\n");
    return 1;
  }
  if (ftp_acl_path_remove_entry(remove_parent_fd, "mkdir-created", 1) != 0) {
    fprintf(stderr, "descriptor-relative MKD fixture cleanup failed\n");
    return 1;
  }
  if (close(remove_parent_fd) != 0) {
    perror("close removal fixture directory");
    return 1;
  }
  checked_path(rename_source, sizeof(rename_source), a, "rename-source.txt");
  checked_path(rename_destination, sizeof(rename_destination), b, "rename-destination.txt");
  checked_path(rename_link, sizeof(rename_link), a, "rename-link");
  checked_path(rename_link_destination, sizeof(rename_link_destination), b,
               "rename-link-moved");
  checked_path(rename_dir, sizeof(rename_dir), a, "rename-directory");
  checked_path(rename_dir_link, sizeof(rename_dir_link), a, "rename-directory-link");
  checked_path(rename_parent_a, sizeof(rename_parent_a), root, "shareA-held-rename");
  checked_path(rename_parent_b, sizeof(rename_parent_b), root, "shareB-held-rename");
  create_fixture_file(rename_source);
  create_fixture_file(rename_destination);
  if (symlink(file_b, rename_link) != 0 || mkdir(rename_dir, 0700) != 0 ||
      symlink(rename_dir, rename_dir_link) != 0) {
    perror("create rename fixtures");
    return 1;
  }
  if (ftp_acl_path_resolve_rename_entry(rename_link, resolved, sizeof(resolved),
                                        existing_leaf, sizeof(existing_leaf)) != 0 ||
      strcmp(resolved, a) || strcmp(existing_leaf, "rename-link")) {
    fprintf(stderr, "rename resolver followed an ordinary final symlink\n");
    return 1;
  }
  if (snprintf(path, sizeof(path), "%s/", rename_dir_link) < 0 ||
      ftp_acl_path_resolve_rename_entry(path, resolved, sizeof(resolved),
          existing_leaf, sizeof(existing_leaf)) != 0 ||
      strcmp(resolved, a) || strcmp(existing_leaf, "rename-directory")) {
    fprintf(stderr, "rename resolver did not follow trailing-slash directory link\n");
    return 1;
  }
  if (ftp_acl_path_resolve_rename_entry(metadata_file, resolved,
          sizeof(resolved), existing_leaf, sizeof(existing_leaf)) == 0 ||
      ftp_acl_path_resolve_rename_entry("/share/.", resolved,
          sizeof(resolved), existing_leaf, sizeof(existing_leaf)) == 0 ||
      ftp_acl_path_resolve_rename_entry("/share/..", resolved,
          sizeof(resolved), existing_leaf, sizeof(existing_leaf)) == 0) {
    fprintf(stderr, "rename resolver accepted metadata or dot entries\n");
    return 1;
  }
  if (snprintf(path, sizeof(path), "%s/", rename_source) < 0 ||
      ftp_acl_path_resolve_rename_entry(path, resolved, sizeof(resolved),
          existing_leaf, sizeof(existing_leaf)) == 0) {
    fprintf(stderr, "rename resolver accepted a trailing slash on a regular file\n");
    return 1;
  }

  rename_source_fd = ftp_acl_path_open_verified_directory(a);
  rename_destination_fd = ftp_acl_path_open_verified_directory(b);
  if (rename_source_fd < 0 || rename_destination_fd < 0 ||
      stat(rename_source, &canonical_st) != 0 ||
      ftp_acl_path_rename_entry(rename_source_fd, "rename-source.txt",
          rename_destination_fd, "rename-destination.txt") != 0 ||
      stat(rename_destination, &st) != 0 ||
      st.st_dev != canonical_st.st_dev || st.st_ino != canonical_st.st_ino) {
    fprintf(stderr, "cross-directory renameat or overwrite semantics failed\n");
    return 1;
  }
  if (lstat(rename_source, &st) == 0) {
    fprintf(stderr, "renameat left its source entry behind\n");
    return 1;
  }
  if (ftp_acl_path_rename_entry(rename_source_fd, "rename-link",
          rename_destination_fd, "rename-link-moved") != 0 ||
      lstat(rename_link_destination, &st) != 0 || !S_ISLNK(st.st_mode) ||
      stat(file_b, &st) != 0 || !S_ISREG(st.st_mode)) {
    fprintf(stderr, "renameat followed the final symlink instead of moving it\n");
    return 1;
  }
  errno = 0;
  if (ftp_acl_path_rename_entry(rename_source_fd, ".__alice_var.txt",
          rename_destination_fd, "not-moved") == 0 || errno != EACCES) {
    fprintf(stderr, "renameat allowed a reserved source leaf\n");
    return 1;
  }
  errno = 0;
  if (ftp_acl_path_rename_entry(rename_source_fd, "rename-directory",
          rename_destination_fd, ".__alice_var.txt") == 0 || errno != EACCES) {
    fprintf(stderr, "renameat allowed a reserved destination leaf\n");
    return 1;
  }
  if (ftp_acl_path_rename_entry(rename_source_fd, ".",
          rename_destination_fd, "invalid-dot") == 0) {
    fprintf(stderr, "renameat allowed a dot source leaf\n");
    return 1;
  }

  {
    int fd_a = openat(rename_source_fd, "pinned-source",
                      O_CREAT | O_EXCL | O_WRONLY | O_NOFOLLOW | O_CLOEXEC, 0600);
    int fd_b = openat(rename_destination_fd, "pinned-source",
                      O_CREAT | O_EXCL | O_WRONLY | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (fd_a < 0 || fd_b < 0 || write(fd_a, "source", 6) != 6 ||
        write(fd_b, "decoy", 5) != 5 || close(fd_a) != 0 || close(fd_b) != 0) {
      perror("create pinned rename source and decoy");
      return 1;
    }
  }
  if (rename(a, rename_parent_a) != 0 || rename(b, rename_parent_b) != 0 ||
      symlink(rename_parent_b, a) != 0 || symlink(rename_parent_a, b) != 0 ||
      ftp_acl_path_rename_entry(rename_source_fd, "pinned-source",
          rename_destination_fd, "pinned-source-moved") != 0) {
    perror("rename through replaced source and destination paths");
    return 1;
  }
  checked_path(path, sizeof(path), rename_parent_a, "pinned-source");
  if (lstat(path, &st) == 0) {
    fprintf(stderr, "renameat used the replaced source pathname\n");
    return 1;
  }
  checked_path(path, sizeof(path), rename_parent_b, "pinned-source-moved");
  if (stat(path, &st) != 0 || st.st_size != 6) {
    fprintf(stderr, "renameat did not use both pinned parent descriptors\n");
    return 1;
  }
  checked_path(path, sizeof(path), rename_parent_b, "pinned-source");
  if (stat(path, &st) != 0 || st.st_size != 5) {
    fprintf(stderr, "renameat moved the decoy from the replaced source path\n");
    return 1;
  }
  if (unlink(path) != 0) {
    perror("remove pinned rename decoy");
    return 1;
  }
  checked_path(path, sizeof(path), rename_parent_b, "pinned-source-moved");
  if (unlink(path) != 0 ||
      unlinkat(rename_source_fd, "rename-directory-link", 0) != 0 ||
      unlinkat(rename_destination_fd, "rename-link-moved", 0) != 0 ||
      unlinkat(rename_destination_fd, "rename-destination.txt", 0) != 0 ||
      unlinkat(rename_source_fd, "rename-directory", AT_REMOVEDIR) != 0) {
    perror("remove rename fixtures through pinned descriptors");
    return 1;
  }
  close(rename_source_fd);
  close(rename_destination_fd);
  if (unlink(a) != 0 || unlink(b) != 0 ||
      rename(rename_parent_a, a) != 0 || rename(rename_parent_b, b) != 0) {
    perror("restore pinned rename directories");
    return 1;
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
  if (ftp_acl_path_resolve_existing_entry(link_file, resolved, sizeof(resolved),
                                           existing_leaf, sizeof(existing_leaf)) ||
      strcmp(resolved, b) || strcmp(existing_leaf, "protected.txt")) {
    fprintf(stderr, "existing final symlink target was not canonicalized\n");
    return 1;
  }
  existing_dir_fd = ftp_acl_path_open_verified_directory(resolved);
  if (existing_dir_fd < 0) {
    perror("pin existing target directory");
    return 1;
  }
  existing_file_fd = ftp_acl_path_open_existing_file(existing_dir_fd,
                                                      existing_leaf, 0);
  if (existing_file_fd < 0 || fstat(existing_file_fd, &st) != 0 ||
      stat(file_b, &canonical_st) != 0 ||
      st.st_dev != canonical_st.st_dev || st.st_ino != canonical_st.st_ino) {
    fprintf(stderr, "descriptor-relative existing target open failed\n");
    return 1;
  }
  close(existing_file_fd);
  existing_file_fd = ftp_acl_path_open_existing_file(existing_dir_fd,
                                                       existing_leaf, 1);
  if (existing_file_fd < 0 || fstat(existing_file_fd, &st) != 0 ||
      (fcntl(existing_file_fd, F_GETFL) & O_PATH) != O_PATH ||
      st.st_dev != canonical_st.st_dev || st.st_ino != canonical_st.st_ino) {
    fprintf(stderr, "stat-only descriptor did not pin the regular target\n");
    return 1;
  }
  close(existing_file_fd);
  close(existing_dir_fd);
  if (ftp_acl_path_resolve_existing_entry(broken_link, resolved,
          sizeof(resolved), existing_leaf, sizeof(existing_leaf)) == 0 ||
      ftp_acl_path_resolve_existing_entry(metadata_alias, resolved,
          sizeof(resolved), existing_leaf, sizeof(existing_leaf)) == 0) {
    fprintf(stderr, "dangling or metadata symlink accepted for existing open\n");
    return 1;
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
  dir_fd = ftp_acl_path_open_verified_directory(a);
  if (dir_fd < 0 || rename(a, held_a) || symlink(b, a)) {
    perror("pin authorized upload directory");
    return 1;
  }
  errno = 0;
  if (ftp_acl_path_verify_directory_identity(a, dir_fd) == 0 ||
      errno != ESTALE) {
    fprintf(stderr, "directory identity check accepted a substituted parent\n");
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
  existing_file_fd = ftp_acl_path_open_existing_file(dir_fd, "race-link", 0);
  if (existing_file_fd >= 0 || errno != EINVAL) {
    fprintf(stderr, "existing-file helper did not reject a substituted final symlink\n");
    return 1;
  }
  errno = 0;
  race_fd = openat(dir_fd, "race-link",
                   O_WRONLY | O_NOFOLLOW | O_CLOEXEC);
  if (race_fd >= 0 || errno != ELOOP) {
    fprintf(stderr, "O_NOFOLLOW accepted a substituted final symlink\n");
    return 1;
  }
  existing_file_fd = ftp_acl_path_open_existing_file(dir_fd, "owned.txt", 0);
  if (existing_file_fd < 0) {
    perror("open existing file through pinned original directory");
    return 1;
  }
  read_length = read(existing_file_fd, readback, sizeof(readback) - 1);
  close(existing_file_fd);
  if (read_length != 8 || memcmp(readback, "fixture\n", 8) != 0) {
    fprintf(stderr, "existing-file open escaped the pinned directory\n");
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

  if (unlink(fifo_file) || unlink(metadata_alias) || unlink(metadata_file) ||
      unlink(dir_alias) || unlink(broken_link) || unlink(link_file) ||
      unlink(file_a) || unlink(file_b) || rmdir(dir_b) ||
      unlink(alias) || rmdir(a) || rmdir(b) || rmdir(root)) {
    perror("remove fixture"); return 1;
  }
  puts("vsftpd ACL path parent policy: PASS");
  return 0;
}

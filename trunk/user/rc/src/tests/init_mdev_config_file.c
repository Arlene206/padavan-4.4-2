#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int injected_fsync(int fd);
static int injected_parent_sync(const char *path);
#define INIT_MDEV_CONFIG_FSYNC(fd) injected_fsync(fd)
#define INIT_MDEV_CONFIG_SYNC_PARENT(path) injected_parent_sync(path)
#include "../init_mdev_config_file.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s (errno=%d)\n", \
                __FILE__, __LINE__, #condition, errno); \
        return 1; \
    } \
} while (0)

static int fsync_calls;
static int fsync_interruptions_remaining;
static int parent_sync_calls;
static int parent_sync_error;

static int
injected_fsync(int fd)
{
    fsync_calls++;
    if (fsync_interruptions_remaining > 0) {
        fsync_interruptions_remaining--;
        errno = EINTR;
        return -1;
    }
    return fsync(fd);
}

static int
injected_parent_sync(const char *path)
{
    parent_sync_calls++;
    if (parent_sync_error != 0) {
        errno = parent_sync_error;
        return -1;
    }
    return config_file_sync_parent(path);
}

static int
write_text_file(const char *path, const char *text)
{
    FILE *fp = fopen(path, "w");
    if (!fp)
        return -1;
    if (fputs(text, fp) == EOF) {
        fclose(fp);
        return -1;
    }
    return fclose(fp);
}

static int
file_equals(const char *path, const char *expected)
{
    char buffer[256];
    FILE *fp = fopen(path, "r");
    size_t length;

    if (!fp)
        return 0;
    length = fread(buffer, 1, sizeof(buffer) - 1, fp);
    if (ferror(fp)) {
        fclose(fp);
        return 0;
    }
    buffer[length] = '\0';
    if (fclose(fp) != 0)
        return 0;
    return strcmp(buffer, expected) == 0;
}

int
main(void)
{
    char directory[] = "/tmp/init-mdev-config.XXXXXX";
    char path[512];
    char staged_path[512];
    char blocked_path[512];
    char sentinel_path[512];
    char failed_stage[512];
    FILE *fp = NULL;
    struct stat st;
    mode_t old_umask;
    int calls_before_publish;

    old_umask = umask(0000);
    CHECK(mkdtemp(directory) != NULL);
    CHECK(snprintf(path, sizeof(path), "%s/mdev.conf", directory) > 0);
    CHECK(snprintf(blocked_path, sizeof(blocked_path), "%s/blocked", directory) > 0);
    CHECK(snprintf(sentinel_path, sizeof(sentinel_path), "%s/keep", blocked_path) > 0);

    CHECK(write_text_file(path, "old-policy\n") == 0);
    CHECK(init_mdev_config_open(path, staged_path, sizeof(staged_path), &fp) == 0);
    CHECK(fp != NULL);
    CHECK(stat(staged_path, &st) == 0);
    CHECK((st.st_mode & 0777) == 0644);
    CHECK(file_equals(path, "old-policy\n"));
    CHECK(fputs("new-policy\n", fp) >= 0);
    CHECK(file_equals(path, "old-policy\n"));
    fsync_interruptions_remaining = 2;
    calls_before_publish = fsync_calls;
    CHECK(init_mdev_config_publish(&fp, staged_path, path) == 0);
    CHECK(fsync_calls - calls_before_publish == 3);
    CHECK(fsync_interruptions_remaining == 0);
    CHECK(parent_sync_calls == 1);
    CHECK(parent_sync_error == 0);
    CHECK(fp == NULL);
    CHECK(file_equals(path, "new-policy\n"));
    CHECK(access(staged_path, F_OK) != 0 && errno == ENOENT);

    CHECK(init_mdev_config_open(path, staged_path, sizeof(staged_path), &fp) == 0);
    CHECK(fputs("directory-sync-error\n", fp) >= 0);
    parent_sync_error = EIO;
    CHECK(init_mdev_config_publish(&fp, staged_path, path) == -EIO);
    CHECK(fp == NULL);
    CHECK(parent_sync_calls == 2);
    /* Rename completed, but the publisher must report the failed durability barrier. */
    CHECK(file_equals(path, "directory-sync-error\n"));
    CHECK(access(staged_path, F_OK) != 0 && errno == ENOENT);
    parent_sync_error = 0;

    /* The shared barrier rejects a missing parent and a symlinked parent. */
    {
        char missing_parent_path[512];
        char symlink_parent[512];
        char symlink_target_path[512];
        CHECK(snprintf(missing_parent_path, sizeof(missing_parent_path),
                       "%s/missing/mdev.conf", directory) > 0);
        CHECK(config_file_sync_parent(missing_parent_path) < 0);
        CHECK(snprintf(symlink_parent, sizeof(symlink_parent),
                       "%s/parent-link", directory) > 0);
        CHECK(symlink(directory, symlink_parent) == 0);
        CHECK(snprintf(symlink_target_path, sizeof(symlink_target_path),
                       "%s/mdev.conf", symlink_parent) > 0);
        CHECK(config_file_sync_parent(symlink_target_path) < 0);
        CHECK(unlink(symlink_parent) == 0);
    }

    CHECK(mkdir(blocked_path, 0700) == 0);
    CHECK(write_text_file(sentinel_path, "preserve\n") == 0);
    CHECK(init_mdev_config_open(blocked_path, failed_stage, sizeof(failed_stage), &fp) == 0);
    CHECK(fputs("must-not-publish\n", fp) >= 0);
    CHECK(init_mdev_config_publish(&fp, failed_stage, blocked_path) < 0);
    CHECK(fp == NULL);
    CHECK(parent_sync_calls == 2);
    CHECK(file_equals(sentinel_path, "preserve\n"));
    CHECK(access(failed_stage, F_OK) != 0 && errno == ENOENT);

    CHECK(unlink(sentinel_path) == 0);
    CHECK(rmdir(blocked_path) == 0);
    CHECK(unlink(path) == 0);
    CHECK(rmdir(directory) == 0);
    umask(old_umask);

    puts("PID 1 mdev config staging, atomic rename, file/directory fsync and no-follow parent checks: PASS");
    return 0;
}

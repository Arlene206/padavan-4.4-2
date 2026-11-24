#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../ralink_config_file.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int
read_file_equals(const char *path, const char *expected)
{
    char buffer[128];
    FILE *fp = fopen(path, "r");
    size_t length;

    if (!fp)
        return 0;
    length = fread(buffer, 1, sizeof(buffer) - 1, fp);
    {
        int failed = ferror(fp);
        if (fclose(fp) != 0)
            failed = 1;
        if (failed)
            return 0;
    }
    buffer[length] = '\0';
    return strcmp(buffer, expected) == 0;
}

static int
test_atomic_replace_and_mode(const char *directory)
{
    char path[256];
    char temp_path[300];
    FILE *fp = NULL;
    struct stat info;

    snprintf(path, sizeof(path), "%s/wireless.dat", directory);
    fp = fopen(path, "w");
    CHECK(fp != NULL);
    CHECK(fputs("old-valid-config\n", fp) >= 0);
    CHECK(fclose(fp) == 0);
    CHECK(chmod(path, 0640) == 0);

    CHECK(ralink_config_open_atomic(path, temp_path, sizeof(temp_path), &fp) == 0);
    CHECK(fp != NULL);
    CHECK(strcmp(path, temp_path) != 0);
    CHECK(strncmp(path, temp_path, strlen(path)) == 0);
    CHECK(stat(temp_path, &info) == 0);
    CHECK((info.st_mode & 0777) == 0640);
    CHECK(fputs("new-complete-config\n", fp) >= 0);
    CHECK(ralink_config_publish_atomic(&fp, temp_path, path) == 0);
    CHECK(fp == NULL);
    CHECK(access(temp_path, F_OK) != 0 && errno == ENOENT);
    CHECK(read_file_equals(path, "new-complete-config\n"));
    CHECK(stat(path, &info) == 0 && (info.st_mode & 0777) == 0640);
    return 0;
}

static int
test_new_file_is_owner_only(const char *directory)
{
    char path[256];
    char temp_path[300];
    FILE *fp = NULL;
    struct stat info;

    snprintf(path, sizeof(path), "%s/new-wireless.dat", directory);
    CHECK(ralink_config_open_atomic(path, temp_path, sizeof(temp_path), &fp) == 0);
    CHECK(stat(temp_path, &info) == 0);
    CHECK((info.st_mode & 0777) == 0600);
    ralink_config_discard_atomic(&fp, temp_path);
    CHECK(access(temp_path, F_OK) != 0 && errno == ENOENT);
    return 0;
}

static int
test_discard_preserves_old_file(const char *directory)
{
    char path[256];
    char temp_path[300];
    FILE *fp = NULL;

    snprintf(path, sizeof(path), "%s/wireless.dat", directory);
    CHECK(ralink_config_open_atomic(path, temp_path, sizeof(temp_path), &fp) == 0);
    CHECK(fputs("unpublished-config\n", fp) >= 0);
    ralink_config_discard_atomic(&fp, temp_path);
    CHECK(fp == NULL);
    CHECK(access(temp_path, F_OK) != 0 && errno == ENOENT);
    CHECK(read_file_equals(path, "new-complete-config\n"));
    return 0;
}

static int
test_rename_failure_cleans_temp(const char *directory)
{
    char path[256];
    char temp_path[300];
    FILE *fp = NULL;

    snprintf(path, sizeof(path), "%s/becomes-directory", directory);
    CHECK(ralink_config_open_atomic(path, temp_path, sizeof(temp_path), &fp) == 0);
    CHECK(fputs("cannot-publish\n", fp) >= 0);
    CHECK(mkdir(path, 0700) == 0);
    CHECK(ralink_config_publish_atomic(&fp, temp_path, path) < 0);
    CHECK(fp == NULL);
    CHECK(access(temp_path, F_OK) != 0 && errno == ENOENT);
    CHECK(access(path, F_OK) == 0);
    CHECK(rmdir(path) == 0);
    return 0;
}

static int
test_path_overflow_has_no_side_effect(const char *directory)
{
    char path[256];
    char temp_path[8];
    FILE *fp = NULL;

    snprintf(path, sizeof(path), "%s/too-long", directory);
    CHECK(ralink_config_open_atomic(path, temp_path, sizeof(temp_path), &fp) < 0);
    CHECK(fp == NULL);
    CHECK(temp_path[0] == '\0');
    CHECK(access(path, F_OK) != 0 && errno == ENOENT);
    return 0;
}

static int
test_delayed_write_error(void)
{
    FILE *fp;

    fp = fopen("/dev/full", "w");
    if (!fp)
        return 0; /* Non-Linux hosts may not provide /dev/full. */
    CHECK(fputs("buffered write must fail on flush\n", fp) >= 0);
    CHECK(ralink_config_finish_stream(fp) < 0);
    return 0;
}


static int
readlink_equals(const char *path, const char *expected)
{
    char value[PATH_MAX];
    ssize_t length = readlink(path, value, sizeof(value) - 1U);

    if (length < 0 || (size_t)length >= sizeof(value) - 1U)
        return 0;
    value[length] = '\0';
    return strcmp(value, expected) == 0;
}

static int
count_sku_staging_directories(const char *directory)
{
    DIR *stream;
    struct dirent *entry;
    int count = 0;

    stream = opendir(directory);
    if (!stream)
        return -1;
    while ((entry = readdir(stream)) != NULL) {
        if (strncmp(entry->d_name, ".ralink-sku-link.", 17U) == 0)
            count++;
    }
    if (closedir(stream) != 0)
        return -1;
    return count;
}

static int
test_symlink_publication_replaces_live_link_atomically(const char *directory)
{
    char old_target[256];
    char new_target[256];
    char link_path[256];
    FILE *fp;

    snprintf(old_target, sizeof(old_target), "%s/sku-old.dat", directory);
    snprintf(new_target, sizeof(new_target), "%s/sku-new.dat", directory);
    snprintf(link_path, sizeof(link_path), "%s/SingleSKU.dat", directory);

    fp = fopen(old_target, "w");
    CHECK(fp != NULL);
    CHECK(fputs("old-sku\n", fp) >= 0);
    CHECK(fclose(fp) == 0);
    fp = fopen(new_target, "w");
    CHECK(fp != NULL);
    CHECK(fputs("new-sku\n", fp) >= 0);
    CHECK(fclose(fp) == 0);
    CHECK(symlink(old_target, link_path) == 0);

    CHECK(ralink_config_publish_symlink_atomic(new_target, link_path) == 0);
    CHECK(readlink_equals(link_path, new_target));
    CHECK(read_file_equals(link_path, "new-sku\n"));
    CHECK(count_sku_staging_directories(directory) == 0);

    CHECK(unlink(link_path) == 0);
    CHECK(unlink(old_target) == 0);
    CHECK(unlink(new_target) == 0);
    return 0;
}

static int
test_symlink_publication_failure_preserves_destination(const char *directory)
{
    char target[256];
    char destination[256];
    struct stat info;
    FILE *fp;

    snprintf(target, sizeof(target), "%s/sku-target.dat", directory);
    snprintf(destination, sizeof(destination), "%s/sku-is-directory", directory);
    fp = fopen(target, "w");
    CHECK(fp != NULL);
    CHECK(fputs("safe-target\n", fp) >= 0);
    CHECK(fclose(fp) == 0);
    CHECK(mkdir(destination, 0700) == 0);

    CHECK(ralink_config_publish_symlink_atomic(target, destination) < 0);
    CHECK(stat(destination, &info) == 0 && S_ISDIR(info.st_mode));
    CHECK(count_sku_staging_directories(directory) == 0);

    CHECK(rmdir(destination) == 0);
    CHECK(unlink(target) == 0);
    return 0;
}

static int
test_optional_unlink_is_idempotent_and_checks_errors(const char *directory)
{
    char missing[256];
    char existing_directory[256];

    snprintf(missing, sizeof(missing), "%s/no-such-sku-link", directory);
    snprintf(existing_directory, sizeof(existing_directory),
             "%s/sku-directory", directory);
    CHECK(ralink_config_unlink_optional(missing) == 0);
    CHECK(mkdir(existing_directory, 0700) == 0);
    CHECK(ralink_config_unlink_optional(existing_directory) < 0);
    CHECK(access(existing_directory, F_OK) == 0);
    CHECK(rmdir(existing_directory) == 0);
    return 0;
}

static int
test_checked_nvram_read_status(void)
{
    CHECK(ralink_config_nvram_read_status(1, 0, "CE") == 0);
    CHECK(ralink_config_nvram_read_status(0, 0, NULL) == 0);
    CHECK(ralink_config_nvram_read_status(-1, EACCES, NULL) == EACCES);
    CHECK(ralink_config_nvram_read_status(-1, 0, NULL) == EIO);
    CHECK(ralink_config_nvram_read_status(2, 0, "CE") == EIO);
    CHECK(ralink_config_nvram_read_status(1, 0, NULL) == EIO);
    return 0;
}

int
main(void)
{
    char directory[] = "/tmp/ralink-config-test-XXXXXX";
    char path[256];

    CHECK(mkdtemp(directory) != NULL);
    CHECK(test_atomic_replace_and_mode(directory) == 0);
    CHECK(test_new_file_is_owner_only(directory) == 0);
    CHECK(test_discard_preserves_old_file(directory) == 0);
    CHECK(test_rename_failure_cleans_temp(directory) == 0);
    CHECK(test_path_overflow_has_no_side_effect(directory) == 0);
    CHECK(test_delayed_write_error() == 0);
    CHECK(test_symlink_publication_replaces_live_link_atomically(directory) == 0);
    CHECK(test_symlink_publication_failure_preserves_destination(directory) == 0);
    CHECK(test_optional_unlink_is_idempotent_and_checks_errors(directory) == 0);
    CHECK(test_checked_nvram_read_status() == 0);

    snprintf(path, sizeof(path), "%s/wireless.dat", directory);
    CHECK(unlink(path) == 0);
    CHECK(rmdir(directory) == 0);
    puts("Ralink wireless config and SKU symlink atomic publication: PASS");
    return 0;
}

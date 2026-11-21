#include <errno.h>
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

    snprintf(path, sizeof(path), "%s/wireless.dat", directory);
    CHECK(unlink(path) == 0);
    CHECK(rmdir(directory) == 0);
    puts("Ralink wireless config atomic publication: PASS");
    return 0;
}

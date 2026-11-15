#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include "msd_lite_config_file.h"

static void write_file(const char *path, const char *contents)
{
    FILE *stream = fopen(path, "w");
    size_t length = strlen(contents);
    assert(stream != NULL);
    assert(fwrite(contents, 1, length, stream) == length);
    assert(fclose(stream) == 0);
}
static void assert_file_equals(const char *path, const char *expected)
{
    char buffer[4096];
    FILE *stream = fopen(path, "r");
    size_t length;
    assert(stream != NULL);
    length = fread(buffer, 1, sizeof(buffer) - 1, stream);
    assert(!ferror(stream));
    assert(feof(stream));
    buffer[length] = '\0';
    assert(fclose(stream) == 0);
    assert(strcmp(buffer, expected) == 0);
}
static void assert_no_temporary_files(const char *directory, const char *prefix)
{
    DIR *stream = opendir(directory);
    struct dirent *entry;
    size_t prefix_length = strlen(prefix);
    assert(stream != NULL);
    while ((entry = readdir(stream)) != NULL)
        assert(strncmp(entry->d_name, prefix, prefix_length) != 0);
    assert(closedir(stream) == 0);
}
int main(void)
{
    char directory_template[] = "/tmp/msd-lite-config.XXXXXX";
    char *directory;
    char source_path[512], destination_path[512], missing_source[512];
    char missing_destination[512], directory_destination[512];
    char symlink_target[512], symlink_path[512];
    const char *source_text =
        "# keep this comment\n"
        "<ifName>old-iface</ifName>\n"
        "<bind><address>0.0.0.0:9000</address><fAcceptFilter>n</fAcceptFilter></bind>\n"
        "<bind><address>[::]:9000</address></bind>\n"
        "<custom>keep-me</custom>\n";
    const char *expected_text =
        "# keep this comment\n"
        "\t\t\t\t<ifName>eth3.2</ifName>\n"
        "\t\t\t<bind><address>0.0.0.0:4022</address><fAcceptFilter>y</fAcceptFilter></bind>\n"
        "\t\t\t<bind><address>[::]:4022</address></bind>\n"
        "<custom>keep-me</custom>\n";
    struct stat st;
    struct rlimit saved_limit, constrained_limit;
    void (*old_sigxfsz)(int);
    int status;
    directory = mkdtemp(directory_template);
    assert(directory != NULL);
    assert(snprintf(source_path, sizeof(source_path), "%s/source.conf", directory) > 0);
    assert(snprintf(destination_path, sizeof(destination_path), "%s/msd_lite.conf", directory) > 0);
    assert(snprintf(missing_source, sizeof(missing_source), "%s/missing.conf", directory) > 0);
    assert(snprintf(missing_destination, sizeof(missing_destination), "%s/no-parent/msd_lite.conf", directory) > 0);
    assert(snprintf(directory_destination, sizeof(directory_destination), "%s/directory-target", directory) > 0);
    assert(snprintf(symlink_target, sizeof(symlink_target), "%s/symlink-target", directory) > 0);
    assert(snprintf(symlink_path, sizeof(symlink_path), "%s/symlink-config", directory) > 0);

    write_file(source_path, source_text);
    assert(chmod(source_path, 0644) == 0);
    write_file(destination_path, "old-config\n");
    assert(chmod(destination_path, 0600) == 0);
    status = msd_lite_config_publish(source_path, destination_path, "eth3.2", 4022);
    assert(status == 0);
    assert_file_equals(destination_path, expected_text);
    assert(stat(destination_path, &st) == 0 && (st.st_mode & 0777) == 0600);
    assert_no_temporary_files(directory, "msd_lite.conf.tmp.");

    write_file(destination_path, "last-good-configuration\n");
    status = msd_lite_config_publish(missing_source, destination_path, "eth3.2", 4022);
    assert(status != 0);
    assert_file_equals(destination_path, "last-good-configuration\n");
    assert(msd_lite_config_publish(source_path, destination_path, "eth3.2\nquickleave", 4022) == EINVAL);
    assert(msd_lite_config_publish(source_path, destination_path, "eth3.2", 70000) == EINVAL);
    assert_file_equals(destination_path, "last-good-configuration\n");
    assert_no_temporary_files(directory, "msd_lite.conf.tmp.");

    status = msd_lite_config_publish(source_path, missing_destination, "eth3.2", 4022);
    assert(status != 0);
    assert_file_equals(destination_path, "last-good-configuration\n");
    assert_no_temporary_files(directory, "msd_lite.conf.tmp.");

    assert(mkdir(directory_destination, 0700) == 0);
    status = msd_lite_config_publish(source_path, directory_destination, "eth3.2", 4022);
    assert(status != 0);
    assert(stat(directory_destination, &st) == 0 && S_ISDIR(st.st_mode));
    assert_no_temporary_files(directory, "directory-target.tmp.");

    write_file(symlink_target, "symlink-target-last-good\n");
    assert(symlink(symlink_target, symlink_path) == 0);
    status = msd_lite_config_publish(source_path, symlink_path, "eth3.2", 4022);
    assert(status == EINVAL);
    assert_file_equals(symlink_target, "symlink-target-last-good\n");
    assert(lstat(symlink_path, &st) == 0 && S_ISLNK(st.st_mode));

    write_file(destination_path, "last-good-after-write-failure\n");
    assert(getrlimit(RLIMIT_FSIZE, &saved_limit) == 0);
    assert(saved_limit.rlim_max == RLIM_INFINITY || saved_limit.rlim_max >= 1);
    constrained_limit = saved_limit;
    constrained_limit.rlim_cur = 1;
    old_sigxfsz = signal(SIGXFSZ, SIG_IGN);
    assert(old_sigxfsz != SIG_ERR);
    assert(setrlimit(RLIMIT_FSIZE, &constrained_limit) == 0);
    status = msd_lite_config_publish(source_path, destination_path, "eth3.2", 4022);
    assert(setrlimit(RLIMIT_FSIZE, &saved_limit) == 0);
    assert(signal(SIGXFSZ, old_sigxfsz) != SIG_ERR);
    assert(status != 0);
    assert_file_equals(destination_path, "last-good-after-write-failure\n");
    assert_no_temporary_files(directory, "msd_lite.conf.tmp.");

    assert(unlink(source_path) == 0);
    assert(unlink(destination_path) == 0);
    assert(unlink(symlink_path) == 0);
    assert(unlink(symlink_target) == 0);
    assert(rmdir(directory_destination) == 0);
    assert(rmdir(directory) == 0);
    puts("MSD Lite atomic configuration publication: PASS");
    return 0;
}

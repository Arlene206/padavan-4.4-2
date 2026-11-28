#ifndef MSD_LITE_CONFIG_FILE_H
#define MSD_LITE_CONFIG_FILE_H
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config_file_sync.h"

static int
msd_lite_config_write(FILE *output, FILE *source, const char *wan_ifname, int port)
{
    char line[512];
    int status;
    if (output == NULL || source == NULL || wan_ifname == NULL ||
        wan_ifname[0] == '\0' || strchr(wan_ifname, '\n') != NULL ||
        strchr(wan_ifname, '\r') != NULL || port < 1024 || port > 65535)
        return EINVAL;
    for (;;) {
        errno = 0;
        if (fgets(line, sizeof(line), source) == NULL)
            break;
        if (strchr(line, '\n') == NULL && !feof(source))
            return EOVERFLOW;
        errno = 0;
        if (strstr(line, "<ifName>"))
            status = fprintf(output, "\t\t\t\t<ifName>%s</ifName>\n", wan_ifname);
        else if (strstr(line, "<bind><address>0.0.0.0:"))
            status = fprintf(output, "\t\t\t<bind><address>0.0.0.0:%d</address><fAcceptFilter>y</fAcceptFilter></bind>\n", port);
        else if (strstr(line, "<bind><address>[::]:"))
            status = fprintf(output, "\t\t\t<bind><address>[::]:%d</address></bind>\n", port);
        else
            status = fprintf(output, "%s", line);
        if (status < 0)
            return errno ? errno : EIO;
    }
    if (ferror(source))
        return errno ? errno : EIO;
    return 0;
}

/*
 * Stage beside the destination. Failures before rename preserve the last-good
 * config; a parent-directory sync failure after rename is reported even
 * though the new configuration is already visible.
 */
static int
msd_lite_config_publish(const char *source_path, const char *destination_path,
                        const char *wan_ifname, int port)
{
    static const char suffix[] = ".tmp.XXXXXX";
    struct stat source_stat, destination_stat;
    char *temporary_path = NULL;
    size_t path_length;
    FILE *source = NULL;
    FILE *output = NULL;
    int source_fd = -1;
    int fd = -1;
    int status = 0;
    mode_t mode;

    if (source_path == NULL || source_path[0] == '\0' ||
        destination_path == NULL || destination_path[0] == '\0' ||
        wan_ifname == NULL || wan_ifname[0] == '\0' ||
        strchr(wan_ifname, '\n') != NULL || strchr(wan_ifname, '\r') != NULL ||
        port < 1024 || port > 65535)
        return EINVAL;
    /*
     * The persisted source may be damaged or replaced with a FIFO. Opening
     * with O_NONBLOCK first prevents startup from waiting for a writer; fstat
     * then validates the pinned inode before stdio can read from it. Following
     * symlinks to regular config files preserves the existing source contract.
     */
    errno = 0;
    source_fd = open(source_path, O_RDONLY | O_NONBLOCK | O_NOCTTY);
    if (source_fd < 0)
        return errno ? errno : EIO;
    if (fstat(source_fd, &source_stat) != 0) {
        status = errno ? errno : EIO;
        goto out;
    }
    if (!S_ISREG(source_stat.st_mode)) {
        status = EINVAL;
        goto out;
    }
    source = fdopen(source_fd, "r");
    if (source == NULL) {
        status = errno ? errno : EIO;
        goto out;
    }
    source_fd = -1;
    if (lstat(destination_path, &destination_stat) == 0) {
        if (!S_ISREG(destination_stat.st_mode)) {
            status = EINVAL;
            goto out;
        }
        mode = destination_stat.st_mode & 0777;
    } else if (errno == ENOENT) {
        mode = source_stat.st_mode & 0777;
    } else {
        status = errno ? errno : EIO;
        goto out;
    }
    path_length = strlen(destination_path);
    if (path_length > (size_t)-1 - sizeof(suffix)) {
        status = ENAMETOOLONG;
        goto out;
    }
    temporary_path = (char *)malloc(path_length + sizeof(suffix));
    if (temporary_path == NULL) {
        status = ENOMEM;
        goto out;
    }
    memcpy(temporary_path, destination_path, path_length);
    memcpy(temporary_path + path_length, suffix, sizeof(suffix));
    fd = mkstemp(temporary_path);
    if (fd < 0) {
        status = errno ? errno : EIO;
        goto out;
    }
    if (fchmod(fd, mode) != 0) {
        status = errno ? errno : EIO;
        goto out;
    }
    output = fdopen(fd, "w");
    if (output == NULL) {
        status = errno ? errno : EIO;
        close(fd);
        fd = -1;
        goto out;
    }
    fd = -1;
    status = msd_lite_config_write(output, source, wan_ifname, port);
    errno = 0;
    if (fclose(source) != 0 && status == 0)
        status = errno ? errno : EIO;
    source = NULL;
    if (status == 0) {
        errno = 0;
        if (fflush(output) != 0)
            status = errno ? errno : EIO;
    }
    if (status == 0 && config_file_sync_fd(fileno(output)) != 0)
        status = errno ? errno : EIO;
    errno = 0;
    if (fclose(output) != 0 && status == 0)
        status = errno ? errno : EIO;
    output = NULL;
    if (status == 0) {
        errno = 0;
        if (rename(temporary_path, destination_path) != 0)
            status = errno ? errno : EIO;
        else if (config_file_sync_parent(destination_path) != 0)
            status = errno ? errno : EIO;
    }
out:
    if (source != NULL)
        fclose(source);
    if (source_fd >= 0)
        close(source_fd);
    if (output != NULL)
        fclose(output);
    if (fd >= 0)
        close(fd);
    if (temporary_path != NULL) {
        if (status != 0)
            unlink(temporary_path);
        free(temporary_path);
    }
    return status;
}
#endif

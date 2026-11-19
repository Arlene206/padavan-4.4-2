#ifndef PADAVAN_L2TP_CONFIG_FILE_H
#define PADAVAN_L2TP_CONFIG_FILE_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define L2TP_CONFIG_PATH_MAX 512
#define L2TP_CONFIG_LEAF_MAX 64

typedef int (*l2tp_config_file_writer_fn)(FILE *fp, void *context);

/*
 * PID 1 deliberately starts with umask(0000). Never create /etc/l2tp with
 * mode 0777: an unprivileged process could plant l2tp.conf as a symlink
 * before the privileged writer opens it. Reject symlinks and foreign owners,
 * and repair permissions through the opened directory descriptor.
 */
static inline int
l2tp_config_directory_prepare(const char *directory)
{
	struct stat st, path_st;
	mode_t safe_mode;
	int fd, flags = O_RDONLY;
	int saved_errno;

	if (!directory || !directory[0]) {
		errno = EINVAL;
		return -1;
	}
#if !defined(O_DIRECTORY) || !defined(O_NOFOLLOW)
	errno = ENOTSUP;
	return -1;
#else
	flags |= O_DIRECTORY | O_NOFOLLOW;
#endif
#ifdef O_CLOEXEC
	flags |= O_CLOEXEC;
#endif
	if (mkdir(directory, 0755) != 0 && errno != EEXIST)
		return -1;

	fd = open(directory, flags);
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISDIR(st.st_mode) || st.st_uid != geteuid()) {
		errno = EPERM;
		goto failed;
	}

	/* Preserve read/execute access but never allow group/other writes. */
	safe_mode = (st.st_mode & 0755) | 0700;
	if ((st.st_mode & 07777) != safe_mode) {
		if (fchmod(fd, safe_mode) != 0)
			goto failed;
		if (fstat(fd, &st) != 0)
			goto failed;
		if (!S_ISDIR(st.st_mode) || st.st_uid != geteuid() ||
		    (st.st_mode & 07777) != safe_mode) {
			errno = EPERM;
			goto failed;
		}
	}

	/* Ensure the pathname still names the directory that was validated. */
	if (lstat(directory, &path_st) != 0)
		goto failed;
	if (!S_ISDIR(path_st.st_mode) || path_st.st_uid != geteuid() ||
	    path_st.st_dev != st.st_dev || path_st.st_ino != st.st_ino) {
		errno = EPERM;
		goto failed;
	}
	if (close(fd) != 0)
		return -1;
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	(void)close(fd);
	errno = saved_errno;
	return -1;
}

/* Same-directory staging prevents symlink following and partial replacement. */
static inline int
l2tp_config_file_publish(const char *directory, const char *filename,
			 l2tp_config_file_writer_fn writer, void *context)
{
	char temporary_path[L2TP_CONFIG_PATH_MAX];
	char target_path[L2TP_CONFIG_PATH_MAX];
	struct stat st;
	FILE *fp = NULL;
	int fd = -1, length, saved_errno;

	if (!directory || !directory[0] || !filename || !filename[0] ||
	    strchr(filename, '/') || !strcmp(filename, ".") ||
	    !strcmp(filename, "..") || strlen(filename) > L2TP_CONFIG_LEAF_MAX ||
	    !writer) {
		errno = EINVAL;
		return -1;
	}
	if (l2tp_config_directory_prepare(directory) != 0)
		return -1;

	length = snprintf(temporary_path, sizeof(temporary_path),
			  "%s/.%s.tmp.XXXXXX", directory, filename);
	if (length < 0 || (size_t)length >= sizeof(temporary_path)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	length = snprintf(target_path, sizeof(target_path), "%s/%s",
			  directory, filename);
	if (length < 0 || (size_t)length >= sizeof(target_path)) {
		errno = ENAMETOOLONG;
		return -1;
	}

	fd = mkstemp(temporary_path);
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() || st.st_nlink != 1) {
		errno = EPERM;
		goto failed;
	}
	fp = fdopen(fd, "w");
	if (!fp)
		goto failed;
	fd = -1;

	if (writer(fp, context) != 0) {
		saved_errno = errno ? errno : EIO;
		goto failed_with_errno;
	}
	if (fflush(fp) != 0 || ferror(fp)) {
		saved_errno = errno ? errno : EIO;
		goto failed_with_errno;
	}
	if (fchmod(fileno(fp), 0644) != 0) {
		saved_errno = errno ? errno : EIO;
		goto failed_with_errno;
	}
	if (fsync(fileno(fp)) != 0) {
		saved_errno = errno ? errno : EIO;
		goto failed_with_errno;
	}
	if (fclose(fp) != 0) {
		fp = NULL;
		saved_errno = errno ? errno : EIO;
		goto failed_with_errno;
	}
	fp = NULL;

	/* rename replaces an old symlink/hardlink without writing through it. */
	if (rename(temporary_path, target_path) != 0)
		goto failed;
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
failed_with_errno:
	if (fp)
		(void)fclose(fp);
	else if (fd >= 0)
		(void)close(fd);
	(void)unlink(temporary_path);
	errno = saved_errno;
	return -1;
}

#endif

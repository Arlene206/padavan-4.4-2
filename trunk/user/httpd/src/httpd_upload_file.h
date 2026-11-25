#ifndef HTTPD_UPLOAD_FILE_H
#define HTTPD_UPLOAD_FILE_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

static inline int
httpd_upload_write_dos2unix(FILE *stream, const char *value)
{
	const char *cursor, *newline;
	size_t line_size, remaining;

	if (!stream || !value) {
		errno = EINVAL;
		return -1;
	}

	cursor = value;
	while ((newline = strstr(cursor, "\r\n")) != NULL) {
		line_size = (size_t)(newline - cursor);
		if (line_size != 0 &&
		    fwrite(cursor, 1, line_size, stream) != line_size)
			return -1;
		if (fputc('\n', stream) == EOF)
			return -1;
		cursor = newline + 2;
	}

	remaining = strlen(cursor);
	if (remaining != 0 &&
	    fwrite(cursor, 1, remaining, stream) != remaining)
		return -1;
	if (remaining != 0 && cursor[remaining - 1] != '\n' &&
	    fputc('\n', stream) == EOF)
		return -1;

	return 0;
}

static FILE *
httpd_upload_open_exclusive(const char *path)
{
	int fd, saved_errno;
	FILE *stream;

	if (!path || !*path) {
		errno = EINVAL;
		return NULL;
	}

	fd = open(path, O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW,
		  S_IRUSR | S_IWUSR);
	if (fd < 0)
		return NULL;

	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0) {
		saved_errno = errno;
		close(fd);
		unlink(path);
		errno = saved_errno;
		return NULL;
	}

	stream = fdopen(fd, "w+");
	if (!stream) {
		saved_errno = errno;
		close(fd);
		unlink(path);
		errno = saved_errno;
		return NULL;
	}

	return stream;
}

typedef struct httpd_upload_file_ops {
	size_t (*write)(FILE *stream, const void *buffer, size_t length);
	int (*flush)(FILE *stream);
	int (*sync)(int fd);
	/* Must release stream even when returning an error, as fclose does. */
	int (*close)(FILE *stream);
	int (*rename)(const char *source, const char *destination);
	int (*unlink)(const char *path);
} httpd_upload_file_ops_t;

static size_t
httpd_upload_default_write(FILE *stream, const void *buffer, size_t length)
{
	return fwrite(buffer, 1, length, stream);
}

static size_t
httpd_upload_write(FILE *stream, const void *buffer, size_t length,
		   const httpd_upload_file_ops_t *ops)
{
	size_t (*write_fn)(FILE *, const void *, size_t);

	if (!stream || (!buffer && length != 0)) {
		errno = EINVAL;
		return 0;
	}

	write_fn = (ops && ops->write) ? ops->write : httpd_upload_default_write;
	return write_fn(stream, buffer, length);
}

static int
httpd_upload_default_flush(FILE *stream)
{
	return fflush(stream);
}

static int
httpd_upload_default_sync(int fd)
{
	return fsync(fd);
}

static int
httpd_upload_default_close(FILE *stream)
{
	return fclose(stream);
}

static int
httpd_upload_default_rename(const char *source, const char *destination)
{
	return rename(source, destination);
}

static int
httpd_upload_default_unlink(const char *path)
{
	return unlink(path);
}

static FILE *
httpd_upload_create_staging(const char *destination, char *path, size_t path_size)
{
	int fd, saved_errno, length;
	FILE *stream;

	if (!destination || !*destination || !path || path_size == 0) {
		errno = EINVAL;
		return NULL;
	}

	length = snprintf(path, path_size, "%s.upload.XXXXXX", destination);
	if (length < 0 || (size_t)length >= path_size) {
		errno = ENAMETOOLONG;
		return NULL;
	}

	fd = mkstemp(path);
	if (fd < 0)
		return NULL;

	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0) {
		saved_errno = errno;
		close(fd);
		unlink(path);
		errno = saved_errno;
		return NULL;
	}

	stream = fdopen(fd, "w+");
	if (!stream) {
		saved_errno = errno;
		close(fd);
		unlink(path);
		errno = saved_errno;
		return NULL;
	}

	return stream;
}

static int
httpd_upload_sync_close(FILE **stream, const httpd_upload_file_ops_t *ops)
{
	FILE *fp;
	int result, saved_errno, fd;
	int (*flush_fn)(FILE *);
	int (*sync_fn)(int);
	int (*close_fn)(FILE *);

	if (!stream || !(fp = *stream)) {
		errno = EINVAL;
		return -1;
	}

	flush_fn = (ops && ops->flush) ? ops->flush : httpd_upload_default_flush;
	sync_fn = (ops && ops->sync) ? ops->sync : httpd_upload_default_sync;
	close_fn = (ops && ops->close) ? ops->close : httpd_upload_default_close;

	result = 0;
	saved_errno = 0;
	fd = fileno(fp);
	if (flush_fn(fp) != 0) {
		result = -1;
		saved_errno = errno;
	} else if (fd < 0 || sync_fn(fd) != 0) {
		result = -1;
		saved_errno = errno;
	}

	if (close_fn(fp) != 0 && result == 0) {
		result = -1;
		saved_errno = errno;
	}
	*stream = NULL;

	if (result != 0)
		errno = (saved_errno != 0) ? saved_errno : EIO;
	return result;
}

static void
httpd_upload_discard_staged(FILE **stream, const char *staging_path,
			    const httpd_upload_file_ops_t *ops)
{
	int saved_errno = errno;
	int (*unlink_fn)(const char *);

	unlink_fn = (ops && ops->unlink) ? ops->unlink : httpd_upload_default_unlink;
	if (stream && *stream)
		httpd_upload_sync_close(stream, ops);
	if (staging_path && *staging_path)
		unlink_fn(staging_path);
	errno = saved_errno;
}

static int
httpd_upload_publish_staged(FILE **stream, const char *staging_path,
			    const char *destination_path,
			    const httpd_upload_file_ops_t *ops)
{
	int saved_errno;
	int (*rename_fn)(const char *, const char *);
	int (*unlink_fn)(const char *);

	rename_fn = (ops && ops->rename) ? ops->rename : httpd_upload_default_rename;
	unlink_fn = (ops && ops->unlink) ? ops->unlink : httpd_upload_default_unlink;

	if (!stream || !*stream || !staging_path || !*staging_path ||
	    !destination_path || !*destination_path) {
		errno = EINVAL;
		if (stream && *stream)
			httpd_upload_sync_close(stream, NULL);
		if (staging_path && *staging_path)
			unlink_fn(staging_path);
		return -1;
	}

	if (httpd_upload_sync_close(stream, ops) != 0) {
		saved_errno = errno;
		unlink_fn(staging_path);
		errno = saved_errno;
		return -1;
	}

	if (rename_fn(staging_path, destination_path) != 0) {
		saved_errno = errno;
		unlink_fn(staging_path);
		errno = saved_errno;
		return -1;
	}

	return 0;
}

#endif

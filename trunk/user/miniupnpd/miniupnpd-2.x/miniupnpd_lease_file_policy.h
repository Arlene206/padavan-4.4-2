#ifndef MINIUPNPD_LEASE_FILE_POLICY_H
#define MINIUPNPD_LEASE_FILE_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define MINIUPNPD_LEASE_RECORD_DESCRIPTION_MAX 63

/* Keep lease records single-line and within reload_from_lease_file()'s
 * 128-byte input buffer. Description text is otherwise opaque. */
static inline int
miniupnpd_lease_record_format(char *buffer, size_t buffer_size,
                              const char *protocol, unsigned short eport,
                              const char *iaddr, unsigned short iport,
                              unsigned int timestamp, const char *description)
{
	char safe_description[MINIUPNPD_LEASE_RECORD_DESCRIPTION_MAX + 1];
	size_t description_length = 0;
	int length;

	if (buffer && buffer_size > 0)
		buffer[0] = '\0';
	if (!buffer || buffer_size == 0 || !protocol || !iaddr) {
		errno = EINVAL;
		return -1;
	}

	if (description) {
		while (description[description_length] != '\0' &&
		       description_length < MINIUPNPD_LEASE_RECORD_DESCRIPTION_MAX) {
			char ch = description[description_length];

			if (ch == '\r' || ch == '\n')
				ch = ' ';
			safe_description[description_length] = ch;
			++description_length;
		}
	}
	safe_description[description_length] = '\0';

	length = snprintf(buffer, buffer_size, "%s:%hu:%s:%hu:%u:%s\n",
	                  protocol, eport, iaddr, iport, timestamp,
	                  safe_description);
	if (length < 0) {
		buffer[0] = '\0';
		errno = EIO;
		return -1;
	}
	if ((size_t)length >= buffer_size) {
		buffer[0] = '\0';
		errno = ENOSPC;
		return -1;
	}
	return length;
}

/* Match a protocol/port key only when the field separator follows the port.
 * This prevents deleting TCP:12345 when removing TCP:1234. */
static inline int
miniupnpd_lease_record_key_matches(const char *record, size_t record_length,
                                   const char *key, size_t key_length)
{
	if (!record || !key || key_length == 0 || record_length <= key_length)
		return 0;
	return memcmp(record, key, key_length) == 0 &&
	       record[key_length] == ':';
}

typedef off_t (*miniupnpd_lease_seek_fn)(int, off_t, int);
typedef ssize_t (*miniupnpd_lease_write_fn)(int, const void *, size_t);
typedef int (*miniupnpd_lease_truncate_fn)(int, off_t);
typedef int (*miniupnpd_lease_sync_fn)(int);

struct miniupnpd_lease_append_ops {
	miniupnpd_lease_seek_fn seek;
	miniupnpd_lease_write_fn write;
	miniupnpd_lease_truncate_fn truncate;
	miniupnpd_lease_sync_fn sync;
};

/* Append a complete record or restore the previous EOF on any write/sync error. */
static inline int
miniupnpd_lease_append_transaction(
	int fd, const char *record, size_t record_length,
	const struct miniupnpd_lease_append_ops *ops, int *rollback_failed)
{
	off_t original_size;
	size_t offset = 0;
	int saved_errno;

	if (rollback_failed)
		*rollback_failed = 0;
	if (fd < 0 || (!record && record_length != 0) || !ops ||
	    !ops->seek || !ops->write || !ops->truncate || !ops->sync) {
		errno = EINVAL;
		return -1;
	}
	original_size = ops->seek(fd, 0, SEEK_END);
	if (original_size < 0)
		return -1;

	while (offset < record_length) {
		ssize_t written = ops->write(fd, record + offset,
		                             record_length - offset);
		if (written < 0) {
			if (errno == EINTR)
				continue;
			goto rollback;
		}
		if (written == 0 || (size_t)written > record_length - offset) {
			errno = EIO;
			goto rollback;
		}
		offset += (size_t)written;
	}
	if (ops->sync(fd) == 0)
		return 0;

rollback:
	saved_errno = errno ? errno : EIO;
	if (ops->truncate(fd, original_size) < 0) {
		if (rollback_failed)
			*rollback_failed = 1;
	} else if (ops->sync(fd) < 0) {
		if (rollback_failed)
			*rollback_failed = 1;
	}
	errno = saved_errno;
	return -1;
}


/* Rewrite one lease key as a single record while preserving all other entries.
 * The caller writes to a temporary file and publishes it with rename(). */
static inline int
miniupnpd_lease_file_rewrite(FILE *source, FILE *destination,
                             const char *key, size_t key_length,
                             const char *replacement,
                             size_t replacement_length)
{
	char record[512];
	size_t record_length;

	if (!destination || !key || key_length == 0 ||
	    (!replacement && replacement_length != 0)) {
		errno = EINVAL;
		return -1;
	}
	while (source && fgets(record, sizeof(record), source) != NULL) {
		record_length = strlen(record);
		if (!miniupnpd_lease_record_key_matches(
			    record, record_length, key, key_length)) {
			errno = 0;
			if (fwrite(record, 1, record_length, destination) != record_length) {
				if (errno == 0)
					errno = EIO;
				return -1;
			}
		}
	}
	if (source && ferror(source)) {
		if (errno == 0)
			errno = EIO;
		return -1;
	}
	if (replacement_length != 0) {
		errno = 0;
		if (fwrite(replacement, 1, replacement_length, destination) !=
		    replacement_length) {
			if (errno == 0)
				errno = EIO;
			return -1;
		}
	}
	return 0;
}

#endif /* MINIUPNPD_LEASE_FILE_POLICY_H */

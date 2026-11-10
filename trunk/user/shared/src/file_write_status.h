#ifndef PADAVAN_FILE_WRITE_STATUS_H
#define PADAVAN_FILE_WRITE_STATUS_H

#include <errno.h>
#include <stdio.h>

/* Write the complete value and report both buffered-write and close failures.
 * This helper owns the stream and always closes it. */
static int
fput_string_stream(FILE *stream, const char *value)
{
	int saved_errno = 0;

	if (!stream || !value) {
		if (stream)
			(void)fclose(stream);
		errno = EINVAL;
		return EINVAL;
	}

	if (fputs(value, stream) == EOF)
		saved_errno = errno ? errno : EIO;

	if (fclose(stream) != 0 && saved_errno == 0)
		saved_errno = errno ? errno : EIO;

	if (saved_errno != 0)
		errno = saved_errno;
	return saved_errno;
}

#endif /* PADAVAN_FILE_WRITE_STATUS_H */

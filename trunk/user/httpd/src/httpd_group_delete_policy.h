#ifndef HTTPD_GROUP_DELETE_POLICY_H
#define HTTPD_GROUP_DELETE_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define HTTPD_GROUP_DELETE_VALUE_SIZE 256

static int
httpd_group_delete_index_parse(const char *text, size_t text_len,
			       int max_index, int *index)
{
	char value[HTTPD_GROUP_DELETE_VALUE_SIZE];
	char *end;
	long parsed;

	if (!text || !index || max_index < 1 || text_len == 0 ||
	    text_len >= sizeof(value))
		return -1;

	/* Preserve the legacy explicit end-of-index-list marker. */
	if (text_len == 2 && text[0] == '-' && text[1] == '1')
		return 1;

	memcpy(value, text, text_len);
	value[text_len] = '\0';

	errno = 0;
	parsed = strtol(value, &end, 10);
	if (end == value || errno == ERANGE || !end || *end != '\0' ||
	    parsed < 0 || parsed >= max_index)
		return -1;

	*index = (int)parsed;
	return 0;
}

#endif /* HTTPD_GROUP_DELETE_POLICY_H */

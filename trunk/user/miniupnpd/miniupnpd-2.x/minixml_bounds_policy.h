#ifndef MINIUPNPD_MINIXML_BOUNDS_POLICY_H
#define MINIUPNPD_MINIXML_BOUNDS_POLICY_H
#include <stddef.h>
#include <string.h>
/* cursor and end must refer to the same object. */
static inline int
miniupnpd_xml_sequence_equal(const char *cursor, const char *end,
                             const char *sequence, size_t length)
{
	if (cursor == NULL || end == NULL || sequence == NULL ||
	    length == 0U || cursor > end)
		return 0;
	return (size_t)(end - cursor) >= length &&
	       memcmp(cursor, sequence, length) == 0;
}
#endif /* MINIUPNPD_MINIXML_BOUNDS_POLICY_H */

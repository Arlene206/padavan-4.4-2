#ifndef BB_PROCPS_STAT_POLICY_H
#define BB_PROCPS_STAT_POLICY_H

#include <stddef.h>
#include <string.h>

/*
 * Locate the command-name field and state separator in one bounded
 * /proc/PID/stat read. The command name is parenthesized but may itself
 * contain spaces and ')' bytes, so only the final ')' is the delimiter.
 *
 * A short/empty/truncated or structurally malformed record is rejected.
 * On failure both output pointers are cleared.
 */
static inline int
procps_stat_comm_bounds(char *buffer, size_t length,
                        char **comm_start, char **comm_end)
{
	char *open_paren;
	char *close_paren = NULL;
	char *cursor;
	char *end;

	if (!comm_start || !comm_end)
		return 0;
	*comm_start = NULL;
	*comm_end = NULL;

	if (!buffer || length == 0 || memchr(buffer, '\0', length) != NULL)
		return 0;

	end = buffer + length;
	open_paren = (char *)memchr(buffer, '(', length);
	if (!open_paren || open_paren == buffer || open_paren[-1] != ' ')
		return 0;

	/* The prefix must be one complete decimal PID, not whitespace or junk. */
	for (cursor = buffer; cursor < open_paren - 1; ++cursor) {
		if (*cursor < '0' || *cursor > '9')
			return 0;
	}
	if (cursor == buffer)
		return 0;

	/* comm may contain ')'; the last one closes the kernel's field. */
	cursor = end;
	while (cursor > open_paren + 1) {
		--cursor;
		if (*cursor == ')') {
			close_paren = cursor;
			break;
		}
	}
	if (!close_paren || (size_t)(end - close_paren) < 4U)
		return 0;

	/* Require the exact ") <state> " boundary before parsing later fields. */
	if (close_paren[1] != ' ' || close_paren[2] == ' ' ||
	    close_paren[2] == '\t' || close_paren[2] == '\n' ||
	    close_paren[3] != ' ')
		return 0;

	*comm_start = open_paren + 1;
	*comm_end = close_paren;
	return 1;
}

/*
 * The optimized top parser uses unchecked pointer walks through fields 4-24
 * (and field 39 when SMP process columns are enabled). A positive short
 * procfs read can still end in the middle of this tail, so validate every
 * token and its delimiter before the fast parser consumes it.
 */
static inline int
procps_stat_tail_fields_valid(const char *tail, size_t length,
                              unsigned int required_fields)
{
	const char *cursor;
	const char *end;
	unsigned int field;

	if (!tail || length == 0 || required_fields == 0 ||
	    memchr(tail, '\0', length) != NULL)
		return 0;

	cursor = tail;
	end = tail + length;
	for (field = 0; field < required_fields; ++field) {
		const char *digits;

		if (cursor >= end)
			return 0;
		if (*cursor == '-' || *cursor == '+')
			++cursor;
		digits = cursor;
		while (cursor < end && *cursor >= '0' && *cursor <= '9')
			++cursor;
		if (cursor == digits || cursor >= end)
			return 0;

		/*
		 * All fields used by fast_strtoul_10()/skip_fields() must
		 * have a real space separator. Newline is only accepted as
		 * the final record delimiter when the required field is last.
		 */
		if (field + 1U == required_fields) {
			if (*cursor != ' ' && *cursor != '\n' && *cursor != '\r')
				return 0;
		} else if (*cursor != ' ') {
			return 0;
		}
		++cursor;
	}
	return 1;
}


#endif /* BB_PROCPS_STAT_POLICY_H */

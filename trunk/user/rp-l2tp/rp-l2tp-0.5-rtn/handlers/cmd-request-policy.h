#ifndef CMD_REQUEST_POLICY_H
#define CMD_REQUEST_POLICY_H

#include <stddef.h>

/*
 * EventTcp_ReadBuf allocates exactly capacity bytes. A request can be
 * terminated only when the callback reports a complete delimiter match,
 * the delimiter is present in the buffer, and one byte remains for NUL.
 */
static inline int
cmd_control_request_is_valid(int complete, size_t length,
			     size_t capacity, int has_newline)
{
	return complete && length > 0 && length < capacity && has_newline;
}

#endif /* CMD_REQUEST_POLICY_H */

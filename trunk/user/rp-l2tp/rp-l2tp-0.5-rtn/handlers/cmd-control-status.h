#ifndef CMD_CONTROL_STATUS_H
#define CMD_CONTROL_STATUS_H

#include <errno.h>
#include <stddef.h>

/*
 * cmd.so replies with a two-token protocol: "OK ..." or "ERR ...".
 * A successful socket write is not proof the requested L2TP operation
 * succeeded; the controller must inspect the daemon's reply.
 */
static inline int
cmd_control_response_status(const char *response, size_t length)
{
	if (!response || length == 0)
		return EPROTO;

	if (length >= 2 && response[0] == 'O' && response[1] == 'K') {
		if (length == 2 || response[2] == ' ' ||
		    response[2] == '\r' || response[2] == '\n')
			return 0;
		return EPROTO;
	}

	if (length >= 3 && response[0] == 'E' &&
	    response[1] == 'R' && response[2] == 'R') {
		if (length == 3 || response[3] == ' ' ||
		    response[3] == '\r' || response[3] == '\n')
			return EIO;
		return EPROTO;
	}

	return EPROTO;
}

#endif /* CMD_CONTROL_STATUS_H */

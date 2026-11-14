#ifndef PPPD_PIDFILE_POLICY_H
#define PPPD_PIDFILE_POLICY_H

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

/* Parse field 22 from Linux /proc/PID/stat. The comm field is parenthesized
 * but may itself contain spaces and right parentheses, so use its last ')'. */
static int
pppd_pidfile_parse_start_time(const char *text, unsigned long long *start_time)
{
	const char *close = NULL, *p, *token;
	unsigned long long value = 0;
	size_t i;

	if (!text || !start_time)
		return -1;
	for (p = text; *p; ++p) {
		if (*p == ')')
			close = p;
	}
	if (!close || close[1] != ' ' || close[2] == '\0' ||
	    close[3] != ' ')
		return -1;

	p = close + 4; /* field 4 (ppid); field 22 is the nineteenth token. */
	for (i = 4; i <= 22; ++i) {
		while (*p == ' ')
			++p;
		token = p;
		while (*p && *p != ' ' && *p != '\n')
			++p;
		if (p == token)
			return -1;
		if (i == 22) {
			const char *digit;
			for (digit = token; digit < p; ++digit) {
				unsigned int n;
				if (*digit < '0' || *digit > '9')
					return -1;
				n = (unsigned int)(*digit - '0');
				if (value > 1844674407370955161ULL ||
				    (value == 1844674407370955161ULL && n > 5))
					return -1;
				value = value * 10 + n;
			}
		}
	}
	if (value == 0)
		return -1;
	*start_time = value;
	return 0;
}

static int
pppd_pidfile_format_record(pid_t pid, unsigned long long start_time,
			   char *buffer, size_t buffer_size)
{
	int length;

	if (pid <= (pid_t)1 || start_time == 0 || !buffer || buffer_size == 0)
		return -1;
	length = snprintf(buffer, buffer_size, "%ld %llu\n",
			  (long)pid, start_time);
	if (length <= 0 || (size_t)length >= buffer_size)
		return -1;
	return length;
}


static int
pppd_pidfile_format_link_record(pid_t pid, const char *ifname,
				unsigned long long start_time,
				char *buffer, size_t buffer_size)
{
	size_t ifname_length = 0, i;
	int length;

	if (pid <= (pid_t)1 || !ifname || start_time == 0 ||
	    !buffer || buffer_size == 0)
		return -1;
	while (ifname[ifname_length]) {
		ifname_length++;
		if (ifname_length > 15)
			return -1;
	}
	if (ifname_length != 0) {
		if (ifname_length < 4 || ifname[0] != 'p' ||
		    ifname[1] != 'p' || ifname[2] != 'p')
			return -1;
		for (i = 3; i < ifname_length; i++) {
			if (ifname[i] < '0' || ifname[i] > '9')
				return -1;
		}
	}
	length = snprintf(buffer, buffer_size, "%ld\n%s\n%llu\n",
			  (long)pid, ifname, start_time);
	if (length <= 0 || (size_t)length >= buffer_size)
		return -1;
	return length;
}

#endif /* PPPD_PIDFILE_POLICY_H */

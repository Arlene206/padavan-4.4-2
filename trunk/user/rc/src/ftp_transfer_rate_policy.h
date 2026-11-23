#ifndef PADAVAN_FTP_TRANSFER_RATE_POLICY_H
#define PADAVAN_FTP_TRANSFER_RATE_POLICY_H

#include <errno.h>

/* Only anonymous-only and mixed anonymous modes consume anon_max_rate. */
static inline int
ftp_transfer_rate_setting_required(int ftp_mode)
{
	return ftp_mode == 3 || ftp_mode == 4;
}

/* Preserve nvram_safe_get_int()'s legacy 0..maximum clamp. A failed checked
 * read must not be translated into zero, which means unlimited in vsftpd. */
static inline int
ftp_transfer_rate_value_status(int read_status, int configured_value,
                               int maximum, int *effective_value)
{
	if (!effective_value || maximum < 0) {
		errno = EINVAL;
		return EINVAL;
	}
	*effective_value = 0;
	if (read_status != 0)
		return read_status > 0 ? read_status : EIO;

	if (configured_value < 0)
		configured_value = 0;
	else if (configured_value > maximum)
		configured_value = maximum;
	*effective_value = configured_value;
	return 0;
}

#endif /* PADAVAN_FTP_TRANSFER_RATE_POLICY_H */

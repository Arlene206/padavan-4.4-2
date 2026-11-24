#include <stdio.h>

#include "ftp_active_port_policy.h"

int
main(void)
{
	if (ftp_active_data_port_is_valid(0U) ||
	    !ftp_active_data_port_is_valid(1U) ||
	    !ftp_active_data_port_is_valid(21U) ||
	    !ftp_active_data_port_is_valid(65535U) ||
	    ftp_active_data_port_is_valid(65536U))
	{
		fprintf(stderr, "vsftpd active-mode TCP port range regression failed\n");
		return 1;
	}
	puts("vsftpd PORT/EPRT nonzero TCP port policy: PASS");
	return 0;
}

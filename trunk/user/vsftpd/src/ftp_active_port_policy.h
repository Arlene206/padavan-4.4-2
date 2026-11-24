#ifndef VSF_FTP_ACTIVE_PORT_POLICY_H
#define VSF_FTP_ACTIVE_PORT_POLICY_H

/* RFC active-mode endpoints must name a nonzero TCP port. */
static inline int
ftp_active_data_port_is_valid(unsigned int port)
{
	return port > 0U && port <= 65535U;
}

#endif /* VSF_FTP_ACTIVE_PORT_POLICY_H */

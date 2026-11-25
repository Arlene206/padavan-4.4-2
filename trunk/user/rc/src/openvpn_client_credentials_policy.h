#ifndef __OPENVPN_CLIENT_CREDENTIALS_POLICY_H__
#define __OPENVPN_CLIENT_CREDENTIALS_POLICY_H__

#include <stddef.h>

/* Volatile stores keep the credential wipe observable to the compiler. */
static inline void
openvpn_client_credentials_secure_clear(void *value, size_t length)
{
	volatile unsigned char *cursor;

	if (!value)
		return;
	cursor = (volatile unsigned char *)value;
	while (length > 0) {
		*cursor++ = 0;
		--length;
	}
}

#endif /* __OPENVPN_CLIENT_CREDENTIALS_POLICY_H__ */

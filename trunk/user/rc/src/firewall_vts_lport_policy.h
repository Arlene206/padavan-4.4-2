#ifndef PADAVAN_FIREWALL_VTS_LPORT_POLICY_H
#define PADAVAN_FIREWALL_VTS_LPORT_POLICY_H

#include <errno.h>

/*
 * Distinguish an absent/empty optional virtual-server translation port from
 * an NVRAM read failure. A failed read must not silently fall back to the
 * external port and publish a different DNAT/accept mapping.
 */
static inline int
firewall_vts_lport_read_status(int read_status, int present,
                               const char *value, int *has_translation)
{
    if (!has_translation)
        return EINVAL;

    if (read_status != 0)
        return read_status > 0 ? read_status : EIO;

    if (present != 0 && present != 1)
        return EINVAL;

    if (present && !value)
        return EIO;

    *has_translation = present && value[0] != '\0';
    return 0;
}

#endif /* PADAVAN_FIREWALL_VTS_LPORT_POLICY_H */

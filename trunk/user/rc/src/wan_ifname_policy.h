#ifndef WAN_IFNAME_POLICY_H
#define WAN_IFNAME_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

/*
 * Copy an NVRAM-backed network interface name only after checking both the
 * kernel-sized destination bound and the token alphabet used by config files.
 * On failure, leave a valid destination empty.
 */
static inline int
wan_ifname_copy(char *destination, size_t destination_size,
                const char *source)
{
        size_t length;
        unsigned char c;

        if (destination == NULL || destination_size == 0)
                return EINVAL;
        destination[0] = '\0';
        if (source == NULL)
                return EINVAL;

        for (length = 0; length < destination_size; ++length) {
                c = (unsigned char)source[length];
                if (c == '\0')
                        break;
                if (!((c >= 'a' && c <= 'z') ||
                      (c >= 'A' && c <= 'Z') ||
                      (c >= '0' && c <= '9') ||
                      c == '_' || c == '-' || c == '.'))
                        return EINVAL;
        }
        if (length == destination_size)
                return ENAMETOOLONG;
        if (length == 0 ||
            (length == 1 && source[0] == '.') ||
            (length == 2 && source[0] == '.' && source[1] == '.'))
                return EINVAL;

        memcpy(destination, source, length + 1);
        return 0;
}

#endif /* WAN_IFNAME_POLICY_H */

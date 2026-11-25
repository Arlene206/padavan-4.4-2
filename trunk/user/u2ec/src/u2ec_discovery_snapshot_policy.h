#ifndef U2EC_DISCOVERY_SNAPSHOT_POLICY_H
#define U2EC_DISCOVERY_SNAPSHOT_POLICY_H

#include <errno.h>
#include <stddef.h>

/*
 * Copy a discovery response's device identity into a bounded destination.
 * The caller provides synchronization when the source is shared state.
 */
static inline int
u2ec_copy_discovery_product(int device_present, const char *source,
                            char *destination, size_t destination_size)
{
    size_t length = 0;

    if (!destination || destination_size == 0 ||
        (device_present && !source)) {
        errno = EINVAL;
        return -1;
    }

    if (!device_present) {
        destination[0] = '\0';
        return 0;
    }

    while (length + 1U < destination_size && source[length] != '\0') {
        destination[length] = source[length];
        length++;
    }
    destination[length] = '\0';
    return 0;
}

#endif /* U2EC_DISCOVERY_SNAPSHOT_POLICY_H */

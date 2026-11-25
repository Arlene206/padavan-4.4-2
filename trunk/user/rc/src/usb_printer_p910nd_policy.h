#ifndef USB_PRINTER_P910ND_POLICY_H
#define USB_PRINTER_P910ND_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

#define USB_PRINTER_P910ND_DEVICE_PREFIX "/dev/usb/lp"

static inline int
usb_printer_p910nd_port_from_device(const char *device, char *port,
                                    size_t port_size)
{
    static const char prefix[] = USB_PRINTER_P910ND_DEVICE_PREFIX;
    size_t prefix_size = sizeof(prefix) - 1;
    char index;

    if (!device || !port) {
        errno = EINVAL;
        return EINVAL;
    }
    if (port_size < 2) {
        errno = ENAMETOOLONG;
        return ENAMETOOLONG;
    }
    if (strncmp(device, prefix, prefix_size) != 0) {
        errno = EINVAL;
        return EINVAL;
    }

    index = device[prefix_size];
    if (index < '0' || index > '9' || device[prefix_size + 1] != '\0') {
        errno = EINVAL;
        return EINVAL;
    }

    port[0] = index;
    port[1] = '\0';
    return 0;
}

/* p910nd rewrites "p910nd" to the matching per-port process argv[0]. */
static inline int
usb_printer_p910nd_process_name(char port, char *name, size_t name_size)
{
    static const char template_name[] = "p9100d";

    if (!name || port < '0' || port > '9') {
        errno = EINVAL;
        return EINVAL;
    }
    if (name_size < sizeof(template_name)) {
        errno = ENAMETOOLONG;
        return ENAMETOOLONG;
    }

    memcpy(name, template_name, sizeof(template_name));
    name[4] = port;
    return 0;
}

#endif /* USB_PRINTER_P910ND_POLICY_H */

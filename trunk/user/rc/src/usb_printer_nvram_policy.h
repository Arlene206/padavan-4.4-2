#ifndef PADAVAN_USB_PRINTER_NVRAM_POLICY_H
#define PADAVAN_USB_PRINTER_NVRAM_POLICY_H
#include <errno.h>
#include <stddef.h>
#include <string.h>
typedef int (*usb_printer_nvram_get_fn)(void *context, const char *name, char **value);
/* Missing keys retain the empty legacy default; cache-backed values are copied immediately. */
static inline int
usb_printer_nvram_copy_setting_status(usb_printer_nvram_get_fn get_value, void *context,
                                      const char *name, char *destination, size_t destination_size)
{
    char *cached_value = NULL;
    size_t length;
    int result, saved_errno;
    if (!get_value || !name || !name[0] || !destination || destination_size == 0)
        return EINVAL;
    destination[0] = '\0';
    errno = 0;
    result = get_value(context, name, &cached_value);
    saved_errno = errno;
    if (result < 0)
        return saved_errno > 0 ? saved_errno : EIO;
    if (result == 0)
        return 0;
    if (result != 1 || !cached_value)
        return EIO;
    length = strlen(cached_value);
    if (length >= destination_size)
        return ENAMETOOLONG;
    memcpy(destination, cached_value, length + 1);
    return 0;
}
/* Normalize nvram_set() errors before reporting printer startup success. */
static inline int
usb_printer_nvram_write_status(int result, int saved_errno)
{
    if (result == 0)
        return 0;
    return saved_errno > 0 ? saved_errno : EIO;
}

/*
 * Printer service selectors are numeric NVRAM modes. Preserve the historical
 * missing-key default (enabled, mode 1), but reject unknown values instead of
 * silently starting a service under an invalid configuration.
 */
static inline int
usb_printer_enable_mode_status(const char *value, int maximum_mode, int *mode)
{
    int parsed_mode;

    if (!value || !mode || (maximum_mode != 1 && maximum_mode != 2)) {
        errno = EINVAL;
        return EINVAL;
    }

    if (value[0] == '\0') {
        parsed_mode = 1;
    } else if (strcmp(value, "0") == 0) {
        parsed_mode = 0;
    } else if (strcmp(value, "1") == 0) {
        parsed_mode = 1;
    } else if (maximum_mode == 2 && strcmp(value, "2") == 0) {
        parsed_mode = 2;
    } else {
        errno = EINVAL;
        return EINVAL;
    }

    *mode = parsed_mode;
    return 0;
}

#endif

#ifndef USB_PRINTER_CHILD_REAP_POLICY_H
#define USB_PRINTER_CHILD_REAP_POLICY_H

#include <errno.h>
#include <sys/types.h>

typedef pid_t (*usb_printer_waitpid_fn)(pid_t child, int *status, int options);

/*
 * Reap a child whose exec failure was reported over the close-on-exec pipe.
 * Retry interruptions so an unrelated signal cannot strand the failed
 * launcher as a zombie. Callers only use this for a child known to have
 * exited (full/partial exec-error payload followed by pipe EOF).
 */
static inline int
usb_printer_reap_failed_exec_child(pid_t child, int *status,
                                   usb_printer_waitpid_fn wait_child)
{
    pid_t result;

    if (child <= 0 || !status || !wait_child) {
        errno = EINVAL;
        return EINVAL;
    }

    do {
        result = wait_child(child, status, 0);
    } while (result < 0 && errno == EINTR);

    if (result == child)
        return 0;

    if (result == 0 || errno <= 0)
        errno = ECHILD;
    return errno;
}

#endif

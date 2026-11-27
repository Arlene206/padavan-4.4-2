#ifndef HEV_EVENT_SOURCE_SIGNAL_TEST_HOOKS_H
#define HEV_EVENT_SOURCE_SIGNAL_TEST_HOOKS_H

#include <signal.h>
#include <stdint.h>

#include "../dns-forwarder-1.2.1/hev-event-source.h"

int dns_event_source_signal_test_signalfd(int fd, const sigset_t *mask,
                                          int flags);
HevEventSourceFD *dns_event_source_signal_test_add_fd(
    HevEventSource *source, int fd, uint32_t events);

#endif /* HEV_EVENT_SOURCE_SIGNAL_TEST_HOOKS_H */

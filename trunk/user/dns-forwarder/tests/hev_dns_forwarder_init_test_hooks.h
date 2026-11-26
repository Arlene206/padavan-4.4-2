#ifndef HEV_DNS_FORWARDER_INIT_TEST_HOOKS_H
#define HEV_DNS_FORWARDER_INIT_TEST_HOOKS_H

int dns_forwarder_test_getaddrinfo(const char *node, const char *service,
                                   const struct addrinfo *hints,
                                   struct addrinfo **result);
void dns_forwarder_test_freeaddrinfo(struct addrinfo *result);
int dns_forwarder_test_socket(int domain, int type, int protocol);
int dns_forwarder_test_ioctl(int fd, unsigned long request, void *argument);
int dns_forwarder_test_setsockopt(int fd, int level, int option,
                                  const void *value, socklen_t length);
int dns_forwarder_test_bind(int fd, const struct sockaddr *address,
                            socklen_t length);
int dns_forwarder_test_close(int fd);

HevEventSource *dns_forwarder_test_event_source_fds_new(void);
HevEventSource *dns_forwarder_test_event_source_timeout_new(unsigned int interval);
void dns_forwarder_test_event_source_set_priority(HevEventSource *source,
                                                   int priority);
HevEventSourceFD *dns_forwarder_test_event_source_add_fd(
    HevEventSource *source, int fd, uint32_t events);
void dns_forwarder_test_event_source_set_callback(
    HevEventSource *source, HevEventSourceFunc callback,
    void *data, HevDestroyNotify notify);
bool dns_forwarder_test_event_loop_add_source(HevEventLoop *loop,
                                               HevEventSource *source);
bool dns_forwarder_test_event_loop_del_source(HevEventLoop *loop,
                                               HevEventSource *source);
HevEventLoop *dns_forwarder_test_event_source_get_loop(HevEventSource *source);
void dns_forwarder_test_event_source_unref(HevEventSource *source);

#endif

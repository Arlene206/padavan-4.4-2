#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../dns-forwarder-1.2.1/hev-dns-forwarder.h"
#include "../dns-forwarder-1.2.1/hev-dns-session.h"
#include <arpa/inet.h>
#include "../dns-forwarder-1.2.1/hev-event-loop.h"
#include "../dns-forwarder-1.2.1/hev-event-source-fds.h"
#include "../dns-forwarder-1.2.1/hev-event-source-timeout.h"
#include "hev_dns_forwarder_init_test_hooks.h"

enum failure_point {
    FAIL_NONE = 0,
    FAIL_GETADDRINFO,
    FAIL_SOCKET,
    FAIL_IOCTL,
    FAIL_SETSOCKOPT,
    FAIL_BIND,
    FAIL_LISTENER_SOURCE_NEW,
    FAIL_LISTENER_SOURCE_ADD_FD,
    FAIL_LISTENER_SOURCE_REGISTER,
    FAIL_TIMEOUT_SOURCE_NEW,
    FAIL_TIMEOUT_SOURCE_REGISTER
};

static enum failure_point failure;
static unsigned int fail_loop_add_call;
static unsigned int getaddrinfo_calls;
static unsigned int freeaddrinfo_calls;
static unsigned int socket_calls;
static unsigned int close_calls;
static unsigned int listener_source_new_calls;
static unsigned int timeout_source_new_calls;
static unsigned int source_add_fd_calls;
static unsigned int loop_add_calls;
static unsigned int loop_del_calls;
static unsigned int source_unref_calls;
static int listener_fd = -1;

enum session_failure_point {
    SESSION_FAIL_NONE = 0,
    SESSION_FAIL_IOCTL,
    SESSION_FAIL_ADD_FD,
    SESSION_FAIL_CONNECT
};

static enum session_failure_point session_failure;
static unsigned int session_socket_calls;
static unsigned int session_ioctl_calls;
static unsigned int session_add_fd_calls;
static unsigned int session_connect_calls;
static unsigned int session_close_notifications;
static int session_remote_fd = -1;

static void
reset_case(enum failure_point selected, unsigned int fail_add_call)
{
    failure = selected;
    fail_loop_add_call = fail_add_call;
    getaddrinfo_calls = 0;
    freeaddrinfo_calls = 0;
    socket_calls = 0;
    close_calls = 0;
    listener_source_new_calls = 0;
    timeout_source_new_calls = 0;
    source_add_fd_calls = 0;
    loop_add_calls = 0;
    loop_del_calls = 0;
    source_unref_calls = 0;
    listener_fd = -1;
}

static int
open_fd_count(void)
{
    int fd;
    int count = 0;

    for (fd = 0; fd < 4096; fd++) {
        if (fcntl(fd, F_GETFD) >= 0)
            count++;
    }
    return count;
}

int
dns_forwarder_test_getaddrinfo(const char *node, const char *service,
                               const struct addrinfo *hints,
                               struct addrinfo **result)
{
    getaddrinfo_calls++;
    if (failure == FAIL_GETADDRINFO) {
        errno = EHOSTUNREACH;
        return EAI_FAIL;
    }
    return getaddrinfo(node, service, hints, result);
}

void
dns_forwarder_test_freeaddrinfo(struct addrinfo *result)
{
    if (result)
        freeaddrinfo_calls++;
    freeaddrinfo(result);
}

int
dns_forwarder_test_socket(int domain, int type, int protocol)
{
    int fd;
    socket_calls++;
    if (failure == FAIL_SOCKET) {
        errno = EMFILE;
        return -1;
    }
    fd = socket(domain, type, protocol);
    if (fd >= 0)
        listener_fd = fd;
    return fd;
}

int
dns_forwarder_test_ioctl(int fd, unsigned long request, void *argument)
{
    if (failure == FAIL_IOCTL) {
        errno = ENOTTY;
        return -1;
    }
    return ioctl(fd, request, argument);
}

int
dns_forwarder_test_setsockopt(int fd, int level, int option,
                              const void *value, socklen_t length)
{
    if (failure == FAIL_SETSOCKOPT) {
        errno = ENOPROTOOPT;
        return -1;
    }
    return setsockopt(fd, level, option, value, length);
}

int
dns_forwarder_test_bind(int fd, const struct sockaddr *address,
                        socklen_t length)
{
    if (failure == FAIL_BIND) {
        errno = EADDRINUSE;
        return -1;
    }
    return bind(fd, address, length);
}

int
dns_forwarder_test_close(int fd)
{
    if (fd == listener_fd)
        close_calls++;
    return close(fd);
}

HevEventSource *
dns_forwarder_test_event_source_fds_new(void)
{
    listener_source_new_calls++;
    if (failure == FAIL_LISTENER_SOURCE_NEW) {
        errno = ENOMEM;
        return NULL;
    }
    return hev_event_source_fds_new();
}

HevEventSource *
dns_forwarder_test_event_source_timeout_new(unsigned int interval)
{
    timeout_source_new_calls++;
    if (failure == FAIL_TIMEOUT_SOURCE_NEW) {
        errno = ENOMEM;
        return NULL;
    }
    return hev_event_source_timeout_new(interval);
}

void
dns_forwarder_test_event_source_set_priority(HevEventSource *source,
                                             int priority)
{
    hev_event_source_set_priority(source, priority);
}

HevEventSourceFD *
dns_forwarder_test_event_source_add_fd(HevEventSource *source, int fd,
                                       uint32_t events)
{
    source_add_fd_calls++;
    if (failure == FAIL_LISTENER_SOURCE_ADD_FD) {
        errno = ENOMEM;
        return NULL;
    }
    return hev_event_source_add_fd(source, fd, events);
}

void
dns_forwarder_test_event_source_set_callback(
    HevEventSource *source, HevEventSourceFunc callback,
    void *data, HevDestroyNotify notify)
{
    hev_event_source_set_callback(source, callback, data, notify);
}

bool
dns_forwarder_test_event_loop_add_source(HevEventLoop *loop,
                                         HevEventSource *source)
{
    loop_add_calls++;
    if (loop_add_calls == fail_loop_add_call &&
        (failure == FAIL_LISTENER_SOURCE_REGISTER ||
         failure == FAIL_TIMEOUT_SOURCE_REGISTER)) {
        errno = EIO;
        return false;
    }
    return hev_event_loop_add_source(loop, source);
}

bool
dns_forwarder_test_event_loop_del_source(HevEventLoop *loop,
                                         HevEventSource *source)
{
    loop_del_calls++;
    return hev_event_loop_del_source(loop, source);
}

HevEventLoop *
dns_forwarder_test_event_source_get_loop(HevEventSource *source)
{
    return hev_event_source_get_loop(source);
}

void
dns_forwarder_test_event_source_unref(HevEventSource *source)
{
    source_unref_calls++;
    hev_event_source_unref(source);
}

int
dns_forwarder_test_session_socket(int domain, int type, int protocol)
{
    int fd;

    session_socket_calls++;
    fd = socket(domain, type, protocol);
    if (fd >= 0 && type == SOCK_STREAM)
        session_remote_fd = fd;
    return fd;
}

int
dns_forwarder_test_session_ioctl(int fd, unsigned long request, void *argument)
{
    session_ioctl_calls++;
    if (session_failure == SESSION_FAIL_IOCTL) {
        errno = ENOTTY;
        return -1;
    }
    return ioctl(fd, request, argument);
}

HevEventSourceFD *
dns_forwarder_test_session_event_source_add_fd(HevEventSource *source,
                                                int fd, uint32_t events)
{
    session_add_fd_calls++;
    if (session_failure == SESSION_FAIL_ADD_FD) {
        errno = ENOMEM;
        return NULL;
    }
    return hev_event_source_add_fd(source, fd, events);
}

int
dns_forwarder_test_session_connect(int fd, const struct sockaddr *address,
                                   socklen_t length)
{
    (void) fd;
    (void) address;
    (void) length;
    session_connect_calls++;
    if (session_failure == SESSION_FAIL_CONNECT) {
        errno = ECONNREFUSED;
        return -1;
    }
    errno = EINPROGRESS;
    return -1;
}

static void
session_test_close_notify(HevDNSSession *session, void *data)
{
    (void) data;
    session_close_notifications++;
    hev_dns_session_unref(session);
}

static int
create_udp_listener_with_query(void)
{
    int listener = -1, sender = -1;
    struct sockaddr_in address;
    socklen_t address_length = sizeof (address);
    unsigned char query[12] = { 0 };

    listener = socket(AF_INET, SOCK_DGRAM, 0);
    if (listener < 0)
        return -1;
    memset(&address, 0, sizeof (address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;
    if (bind(listener, (struct sockaddr *) &address, sizeof (address)) < 0)
        goto error;
    if (getsockname(listener, (struct sockaddr *) &address, &address_length) < 0)
        goto error;
    sender = socket(AF_INET, SOCK_DGRAM, 0);
    if (sender < 0)
        goto error;
    if (sendto(sender, query, sizeof (query), 0,
               (struct sockaddr *) &address, sizeof (address)) !=
        (ssize_t) sizeof (query))
        goto error;
    close(sender);
    return listener;

error:
    if (sender >= 0)
        close(sender);
    close(listener);
    return -1;
}

static void
run_session_connect_failure(enum session_failure_point selected,
                            unsigned int expected_ioctl,
                            unsigned int expected_add_fd,
                            unsigned int expected_connect)
{
    unsigned int before = (unsigned int) open_fd_count();
    struct sockaddr_in upstream;
    HevDNSSession *session = NULL;
    int listener = create_udp_listener_with_query();

    assert(listener >= 0);
    memset(&upstream, 0, sizeof (upstream));
    upstream.sin_family = AF_INET;
    upstream.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    upstream.sin_port = htons(53);

    session_failure = selected;
    session_socket_calls = 0;
    session_ioctl_calls = 0;
    session_add_fd_calls = 0;
    session_connect_calls = 0;
    session_close_notifications = 0;
    session_remote_fd = -1;

    session = hev_dns_session_new(listener, &upstream,
                                  session_test_close_notify, NULL);
    assert(session != NULL);
    assert(hev_dns_session_get_source(session) != NULL);
    hev_dns_session_start(session);

    assert(session_close_notifications == 1);
    assert(session_socket_calls == 1);
    assert(session_ioctl_calls == expected_ioctl);
    assert(session_add_fd_calls == expected_add_fd);
    assert(session_connect_calls == expected_connect);
    assert(session_remote_fd >= 0);
    errno = 0;
    assert(fcntl(session_remote_fd, F_GETFD) == -1);
    assert(errno == EBADF);

    close(listener);
    session_failure = SESSION_FAIL_NONE;
    assert((unsigned int) open_fd_count() == before);
}

static void
test_session_connect_setup_failures(void)
{
    run_session_connect_failure(SESSION_FAIL_IOCTL, 1, 0, 0);
    run_session_connect_failure(SESSION_FAIL_ADD_FD, 1, 1, 0);
    run_session_connect_failure(SESSION_FAIL_CONNECT, 1, 1, 1);
}

static void
expect_failure(HevEventLoop *loop, enum failure_point selected,
               unsigned int fail_add_call, int expected_errno,
               unsigned int expected_freeaddrinfo,
               unsigned int expected_close, unsigned int expected_loop_add,
               unsigned int expected_loop_del, unsigned int expected_unref,
               unsigned int expected_timeout_new)
{
    unsigned int before = (unsigned int) open_fd_count();
    HevDNSForwarder *forwarder;

    reset_case(selected, fail_add_call);
    errno = 0;
    forwarder = hev_dns_forwarder_new(loop, "127.0.0.1", "0",
                                      "8.8.8.8", "53");
    assert(forwarder == NULL);
    assert(errno == expected_errno);
    assert(getaddrinfo_calls == 1);
    assert(freeaddrinfo_calls == expected_freeaddrinfo);
    assert(socket_calls == (selected == FAIL_GETADDRINFO ? 0U : 1U));
    assert(close_calls == expected_close);
    assert(listener_source_new_calls ==
           (selected >= FAIL_LISTENER_SOURCE_NEW ? 1U : 0U));
    assert(source_add_fd_calls ==
           (selected >= FAIL_LISTENER_SOURCE_ADD_FD ? 1U : 0U));
    assert(loop_add_calls == expected_loop_add);
    assert(loop_del_calls == expected_loop_del);
    assert(source_unref_calls == expected_unref);
    assert(timeout_source_new_calls == expected_timeout_new);
    assert((unsigned int) open_fd_count() == before);
}

int
main(void)
{
    HevEventLoop *loop = hev_event_loop_new();
    HevDNSForwarder *forwarder;
    unsigned int before;

    assert(loop != NULL);

    expect_failure(loop, FAIL_GETADDRINFO, 0, EINVAL, 0, 0, 0, 0, 0, 0);
    expect_failure(loop, FAIL_SOCKET, 0, EMFILE, 1, 0, 0, 0, 0, 0);
    expect_failure(loop, FAIL_IOCTL, 0, ENOTTY, 1, 1, 0, 0, 0, 0);
    expect_failure(loop, FAIL_SETSOCKOPT, 0, ENOPROTOOPT, 1, 1, 0, 0, 0, 0);
    expect_failure(loop, FAIL_BIND, 0, EADDRINUSE, 1, 1, 0, 0, 0, 0);
    expect_failure(loop, FAIL_LISTENER_SOURCE_NEW, 0, ENOMEM, 1, 1, 0, 0, 0, 0);
    expect_failure(loop, FAIL_LISTENER_SOURCE_ADD_FD, 0, ENOMEM, 1, 1, 0, 0, 1, 0);
    expect_failure(loop, FAIL_LISTENER_SOURCE_REGISTER, 1, EIO, 1, 1, 1, 0, 1, 0);
    expect_failure(loop, FAIL_TIMEOUT_SOURCE_NEW, 0, ENOMEM, 1, 1, 1, 1, 1, 1);
    expect_failure(loop, FAIL_TIMEOUT_SOURCE_REGISTER, 2, EIO, 1, 1, 2, 1, 2, 1);

    reset_case(FAIL_NONE, 0);
    before = (unsigned int) open_fd_count();
    errno = 0;
    forwarder = hev_dns_forwarder_new(loop, "127.0.0.1", "0",
                                      "8.8.8.8", "53");
    assert(forwarder != NULL);
    assert(getaddrinfo_calls == 1);
    assert(freeaddrinfo_calls == 1);
    assert(socket_calls == 1);
    assert(close_calls == 0);
    assert(listener_source_new_calls == 1);
    assert(timeout_source_new_calls == 1);
    assert(source_add_fd_calls == 1);
    assert(loop_add_calls == 2);
    assert(loop_del_calls == 0);
    assert((unsigned int) open_fd_count() == before + 2);

    hev_dns_forwarder_unref(forwarder);
    assert(close_calls == 1);
    assert(loop_del_calls == 2);
    assert((unsigned int) open_fd_count() == before);

    test_session_connect_setup_failures();

    hev_event_loop_unref(loop);
    puts("DNS Forwarder constructor syscall/source rollback regression: PASS");
    return 0;
}

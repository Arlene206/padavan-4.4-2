#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "../detect_wan_arp_socket_policy.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int calls;

static int
test_device_name_is_bounded_and_copied_without_truncation(void)
{
    struct ifreq request;
    struct sockaddr address;

    memset(&request, 0xa5, sizeof(request));
    memset(&address, 0xa5, sizeof(address));
    CHECK(detect_wan_arp_socket_device_name_prepare(
              "eth0", &request, &address) == 0);
    CHECK(strcmp(request.ifr_name, "eth0") == 0);
    CHECK(strcmp(address.sa_data, "eth0") == 0);
    return 0;
}

static int
test_invalid_device_names_fail_without_partial_output(void)
{
    static const char too_long_for_sock_packet[] = "abcdefghijklmn";
    struct ifreq request, before_request;
    struct sockaddr address, before_address;

    memset(&request, 0x5a, sizeof(request));
    memset(&address, 0xa5, sizeof(address));
    memcpy(&before_request, &request, sizeof(request));
    memcpy(&before_address, &address, sizeof(address));

    CHECK(detect_wan_arp_socket_device_name_prepare(
              "", &request, &address) == -1);
    CHECK(errno == ENODEV);
    CHECK(memcmp(&request, &before_request, sizeof(request)) == 0);
    CHECK(memcmp(&address, &before_address, sizeof(address)) == 0);

    CHECK(detect_wan_arp_socket_device_name_prepare(
              too_long_for_sock_packet, &request, &address) == -1);
    CHECK(errno == ENAMETOOLONG);
    CHECK(memcmp(&request, &before_request, sizeof(request)) == 0);
    CHECK(memcmp(&address, &before_address, sizeof(address)) == 0);

    CHECK(detect_wan_arp_socket_device_name_prepare(
              NULL, &request, &address) == -1);
    CHECK(errno == EINVAL);
    CHECK(memcmp(&request, &before_request, sizeof(request)) == 0);
    CHECK(memcmp(&address, &before_address, sizeof(address)) == 0);
    return 0;
}
static int fail_option;
static int bad_arguments;

static int
mock_setsockopt(int fd, int level, int option, const void *value, socklen_t length)
{
    calls++;
    if (fd != 7 || level != SOL_SOCKET)
        bad_arguments = 1;

    if (option == SO_BINDTODEVICE) {
        const struct ifreq *request = (const struct ifreq *)value;
        if (length != sizeof(*request) || strcmp(request->ifr_name, "eth0") != 0)
            bad_arguments = 1;
    } else if (option == SO_RCVTIMEO) {
        const struct timeval *timeout = (const struct timeval *)value;
        if (length != sizeof(*timeout) || timeout->tv_sec != 1 || timeout->tv_usec != 0)
            bad_arguments = 1;
    } else {
        bad_arguments = 1;
    }

    if (option == fail_option) {
        errno = EACCES;
        return -1;
    }
    return 0;
}

static void
reset_mock(int option)
{
    calls = 0;
    fail_option = option;
    bad_arguments = 0;
}

static int
test_bind_failure_prevents_timeout_setup(void)
{
    struct ifreq request;
    struct timeval timeout = { 1, 0 };

    memset(&request, 0, sizeof(request));
    strcpy(request.ifr_name, "eth0");
    reset_mock(SO_BINDTODEVICE);
    CHECK(detect_wan_arp_socket_prepare(7, &request, &timeout, mock_setsockopt) == -1);
    CHECK(errno == EACCES);
    CHECK(calls == 1);
    CHECK(!bad_arguments);
    return 0;
}

static int
test_timeout_failure_is_reported(void)
{
    struct ifreq request;
    struct timeval timeout = { 1, 0 };

    memset(&request, 0, sizeof(request));
    strcpy(request.ifr_name, "eth0");
    reset_mock(SO_RCVTIMEO);
    CHECK(detect_wan_arp_socket_prepare(7, &request, &timeout, mock_setsockopt) == -1);
    CHECK(errno == EACCES);
    CHECK(calls == 2);
    CHECK(!bad_arguments);
    return 0;
}

static int
test_success_installs_both_prerequisites(void)
{
    struct ifreq request;
    struct timeval timeout = { 1, 0 };

    memset(&request, 0, sizeof(request));
    strcpy(request.ifr_name, "eth0");
    reset_mock(0);
    CHECK(detect_wan_arp_socket_prepare(7, &request, &timeout, mock_setsockopt) == 0);
    CHECK(calls == 2);
    CHECK(!bad_arguments);
    return 0;
}

static int
test_invalid_arguments_fail_without_syscalls(void)
{
    struct ifreq request;
    struct timeval timeout = { 1, 0 };

    memset(&request, 0, sizeof(request));
    reset_mock(0);
    CHECK(detect_wan_arp_socket_prepare(-1, &request, &timeout, mock_setsockopt) == -1);
    CHECK(errno == EINVAL && calls == 0);
    CHECK(detect_wan_arp_socket_prepare(7, NULL, &timeout, mock_setsockopt) == -1);
    CHECK(errno == EINVAL && calls == 0);
    CHECK(detect_wan_arp_socket_prepare(7, &request, NULL, mock_setsockopt) == -1);
    CHECK(errno == EINVAL && calls == 0);
    CHECK(detect_wan_arp_socket_prepare(7, &request, &timeout, NULL) == -1);
    CHECK(errno == EINVAL && calls == 0);

    CHECK(detect_wan_arp_socket_prepare(7, &request, &timeout, NULL) == -1);
    CHECK(errno == EINVAL && calls == 0);

    memset(&request, 0, sizeof(request));
    reset_mock(0);
    CHECK(detect_wan_arp_socket_prepare(7, &request, &timeout, mock_setsockopt) == -1);
    CHECK(errno == ENODEV && calls == 0);

    memset(request.ifr_name, 'x', sizeof(request.ifr_name));
    reset_mock(0);
    CHECK(detect_wan_arp_socket_prepare(7, &request, &timeout, mock_setsockopt) == -1);
    CHECK(errno == ENAMETOOLONG && calls == 0);

    /* Also compile-check compatibility with the platform setsockopt API. */
    CHECK(detect_wan_arp_socket_prepare(-1, &request, &timeout, setsockopt) == -1);
    CHECK(errno == EINVAL && calls == 0);
    return 0;
}

int
main(void)
{
    CHECK(test_device_name_is_bounded_and_copied_without_truncation() == 0);
    CHECK(test_invalid_device_names_fail_without_partial_output() == 0);
    CHECK(test_bind_failure_prevents_timeout_setup() == 0);
    CHECK(test_timeout_failure_is_reported() == 0);
    CHECK(test_success_installs_both_prerequisites() == 0);
    CHECK(test_invalid_arguments_fail_without_syscalls() == 0);
    puts("WAN ARP socket setup failure policy: PASS");
    return 0;
}

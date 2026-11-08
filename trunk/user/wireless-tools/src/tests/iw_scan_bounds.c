/*
 * Focused regression tests for Wireless Extensions scan bounds.
 */
#include <stdio.h>
#include <string.h>

#include "iwlib.h"
#include "iw_scan_bounds.h"

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "scan bounds regression failed at line %d: %s\n", \
                    __LINE__, #condition); \
            return 1; \
        } \
    } while (0)

static void
put_u16(unsigned char *buffer, __u16 value)
{
    memcpy(buffer, &value, sizeof(value));
}

static int
test_length_policies(void)
{
    CHECK(iw_scan_essid_length_valid(0, IW_ESSID_MAX_SIZE));
    CHECK(iw_scan_essid_length_valid(IW_ESSID_MAX_SIZE,
                                     IW_ESSID_MAX_SIZE));
    CHECK(!iw_scan_essid_length_valid(IW_ESSID_MAX_SIZE + 1,
                                      IW_ESSID_MAX_SIZE));
    CHECK(iw_scan_event_length_valid(20, 20, 8));
    CHECK(!iw_scan_event_length_valid(19, 20, 8));
    CHECK(!iw_scan_event_length_valid(20, 7, 8));
    return 0;
}

static int
test_truncated_fixed_event(void)
{
    unsigned char frame[IW_EV_ADDR_PK_LEN];
    struct stream_descr stream;
    struct iw_event iwe;
    struct sockaddr address;
    int result;

    memset(frame, 0x5a, sizeof(frame));
    memset(&address, 0, sizeof(address));
    address.sa_family = ARPHRD_ETHER;
    memcpy(frame + IW_EV_LCP_PK_LEN, &address, sizeof(address));
    put_u16(frame, IW_EV_LCP_PK_LEN + sizeof(__u32));
    put_u16(frame + sizeof(__u16), SIOCGIWAP);

    iw_init_event_stream(&stream, (char *) frame, sizeof(frame));
    result = iw_extract_event_stream(&stream, &iwe, WE_VERSION);
    CHECK(result < 0);
    CHECK(stream.value == NULL);
    CHECK((unsigned char *) stream.current == frame + IW_EV_LCP_PK_LEN + sizeof(__u32));
    return 0;
}

static int
test_point_payload_cannot_exceed_buffer(void)
{
    unsigned char frame[IW_EV_POINT_PK_LEN + 4];
    struct stream_descr stream;
    struct iw_event iwe;
    int result;

    memset(frame, 0, sizeof(frame));
    put_u16(frame, IW_EV_POINT_PK_LEN + 32);
    put_u16(frame + sizeof(__u16), SIOCGIWESSID);
    put_u16(frame + IW_EV_LCP_PK_LEN, 32);
    put_u16(frame + IW_EV_LCP_PK_LEN + sizeof(__u16), 1);
    memcpy(frame + IW_EV_POINT_PK_LEN, "test", 4);

    iw_init_event_stream(&stream, (char *) frame, sizeof(frame));
    result = iw_extract_event_stream(&stream, &iwe, WE_VERSION);
    CHECK(result < 0);
    CHECK(stream.value == NULL);
    CHECK((unsigned char *) stream.current == frame + sizeof(frame));
    return 0;
}

static int
test_valid_point_payload(void)
{
    unsigned char frame[IW_EV_POINT_PK_LEN + 3];
    struct stream_descr stream;
    struct iw_event iwe;
    int result;

    memset(frame, 0, sizeof(frame));
    put_u16(frame, sizeof(frame));
    put_u16(frame + sizeof(__u16), SIOCGIWESSID);
    put_u16(frame + IW_EV_LCP_PK_LEN, 3);
    put_u16(frame + IW_EV_LCP_PK_LEN + sizeof(__u16), 1);
    memcpy(frame + IW_EV_POINT_PK_LEN, "abc", 3);

    iw_init_event_stream(&stream, (char *) frame, sizeof(frame));
    result = iw_extract_event_stream(&stream, &iwe, WE_VERSION);
    CHECK(result == 1);
    CHECK(iwe.u.data.pointer != NULL);
    CHECK(iwe.u.data.length == 3);
    CHECK((char *) iwe.u.data.pointer >= (char *) frame);
    CHECK(((char *) iwe.u.data.pointer + iwe.u.data.length) <=
          ((char *) frame + sizeof(frame)));
    CHECK(memcmp(iwe.u.data.pointer, "abc", 3) == 0);
    CHECK((unsigned char *) stream.current == frame + sizeof(frame));
    return 0;
}

int
main(void)
{
    CHECK(test_length_policies() == 0);
    CHECK(test_truncated_fixed_event() == 0);
    CHECK(test_point_payload_cannot_exceed_buffer() == 0);
    CHECK(test_valid_point_payload() == 0);
    puts("Wireless scan event and ESSID bounds: PASS");
    return 0;
}

#include <stdio.h>
#include <string.h>

#include "nvram_profile_policy.h"

static int expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

static int test_headers(void)
{
    unsigned char hdr1[NVRAM_PROFILE_HEADER_SIZE] = {
        'H', 'D', 'R', '1', 0x00, 0x04, 0x00, 0x00
    };
    unsigned char hdr2[NVRAM_PROFILE_HEADER_SIZE] = {
        'H', 'D', 'R', '2', 0x00, 0x04, 0x00, 0x2a
    };
    unsigned long length = 0;
    unsigned char random_value = 0;
    int current = -1;

    if (!expect(nvram_profile_parse_header(hdr1, 4096, 5120,
                                           &length, &random_value, &current),
                "HDR1 header accepted"))
        return 0;
    if (!expect(length == 1024 && random_value == 0 && current == 0,
                "HDR1 length and format decoded"))
        return 0;

    if (!expect(nvram_profile_parse_header(hdr2, 4096, 5120,
                                           &length, &random_value, &current),
                "HDR2 header accepted"))
        return 0;
    if (!expect(length == 1024 && random_value == 0x2a && current == 1,
                "HDR2 length and random byte decoded"))
        return 0;

    if (!expect(!nvram_profile_parse_header(hdr1, 1023, 5120,
                                            &length, &random_value, &current),
                "legacy length above buffer rejected"))
        return 0;
    hdr1[4] = hdr1[5] = hdr1[6] = hdr1[7] = 0;
    if (!expect(!nvram_profile_parse_header(hdr1, 4096, 5120,
                                            &length, &random_value, &current),
                "zero-length legacy profile rejected"))
        return 0;
    hdr1[5] = 0x04;
    hdr2[4] = hdr2[5] = hdr2[6] = 0;
    if (!expect(!nvram_profile_parse_header(hdr2, 4096, 5120,
                                            &length, &random_value, &current),
                "zero-length current profile rejected"))
        return 0;
    hdr2[4] = 0xff;
    hdr2[5] = 0xff;
    hdr2[6] = 0xff;
    if (!expect(!nvram_profile_parse_header(hdr2, 4096, 5120,
                                            &length, &random_value, &current),
                "oversized current-format length rejected"))
        return 0;
    hdr2[0] = 'X';
    if (!expect(!nvram_profile_parse_header(hdr2, 4096, 5120,
                                            &length, &random_value, &current),
                "unknown profile header rejected"))
        return 0;
    if (!expect(!nvram_profile_parse_header(NULL, 4096, 5120,
                                            &length, &random_value, &current),
                "NULL header rejected"))
        return 0;

    return 1;
}

static int test_payloads(void)
{
    static const unsigned char valid[] = "lan_ipaddr=192.168.1.1\0empty=\0\0\0";
    static const unsigned char empty[] = { 0, 0, 0, 0 };
    static const unsigned char missing_terminator[] = { 'x', '=', '1' };
    static const unsigned char missing_empty_record[] = { 'x', '=', '1', 0 };
    static const unsigned char missing_equal[] = { 'x', 'y', 0, 0 };
    static const unsigned char empty_name[] = { '=', '1', 0, 0 };
    static const unsigned char dirty_padding[] = { 'x', '=', '1', 0, 0, 'z' };
    unsigned char long_name[67];
    unsigned char long_value[8];

    if (!expect(nvram_profile_payload_valid(valid, sizeof(valid), 63, 4095),
                "valid profile list and zero padding accepted"))
        return 0;
    if (!expect(nvram_profile_payload_valid(empty, sizeof(empty), 63, 4095),
                "empty profile accepted"))
        return 0;
    if (!expect(!nvram_profile_payload_valid(missing_terminator,
                                              sizeof(missing_terminator), 63, 4095),
                "unterminated record rejected"))
        return 0;
    if (!expect(!nvram_profile_payload_valid(missing_empty_record,
                                              sizeof(missing_empty_record), 63, 4095),
                "missing empty-record terminator rejected"))
        return 0;
    if (!expect(!nvram_profile_payload_valid(missing_equal,
                                              sizeof(missing_equal), 63, 4095),
                "record without equals rejected"))
        return 0;
    if (!expect(!nvram_profile_payload_valid(empty_name,
                                              sizeof(empty_name), 63, 4095),
                "empty variable name rejected"))
        return 0;
    if (!expect(!nvram_profile_payload_valid(dirty_padding,
                                              sizeof(dirty_padding), 63, 4095),
                "nonzero bytes after empty record rejected"))
        return 0;

    memset(long_name, 'n', sizeof(long_name));
    long_name[64] = '=';
    long_name[65] = 'v';
    long_name[66] = 0;
    if (!expect(!nvram_profile_payload_valid(long_name, sizeof(long_name), 63, 4095),
                "oversized variable name rejected"))
        return 0;

    memset(long_value, 'v', sizeof(long_value));
    long_value[0] = 'x';
    long_value[1] = '=';
    long_value[7] = 0;
    if (!expect(!nvram_profile_payload_valid(long_value, sizeof(long_value), 63, 4),
                "oversized variable value rejected"))
        return 0;

    if (!expect(!nvram_profile_payload_valid(NULL, 1, 63, 4095),
                "NULL non-empty payload rejected"))
        return 0;
    if (!expect(nvram_profile_payload_valid(NULL, 0, 63, 4095),
                "zero-length payload accepted"))
        return 0;

    return 1;
}

static int test_decode(void)
{
    unsigned char payload[] = { 0xa0, 0xc4, 0x89, 0xfd, 0xfe };

    nvram_profile_decode_payload(payload, sizeof(payload), 2);
    return expect(payload[0] == 'a' && payload[1] == '=' &&
                  payload[2] == 'x' && payload[3] == 0 && payload[4] == 0,
                  "HDR2 decoding preserves entries and padding NULs");
}

int main(void)
{
    if (!test_headers() || !test_payloads() || !test_decode())
        return 1;

    puts("NVRAM profile policy: PASS");
    return 0;
}

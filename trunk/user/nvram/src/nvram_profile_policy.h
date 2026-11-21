#ifndef NVRAM_PROFILE_POLICY_H
#define NVRAM_PROFILE_POLICY_H

#include <stddef.h>
#include <string.h>

#define NVRAM_PROFILE_HEADER_SIZE 8
#define NVRAM_PROFILE_HDR1 "HDR1"
#define NVRAM_PROFILE_HDR2 "HDR2"

/* The on-disk legacy length is a 32-bit little-endian value on MIPS. */
static int nvram_profile_parse_header(const unsigned char *header,
                                      unsigned long legacy_limit,
                                      unsigned long current_limit,
                                      unsigned long *file_length,
                                      unsigned char *random_value,
                                      int *is_current_format)
{
    unsigned long length;

    if (!header || !file_length || !random_value || !is_current_format)
        return 0;

    if (memcmp(header, NVRAM_PROFILE_HDR1, 4) == 0) {
        length = (unsigned long)header[4] |
                 ((unsigned long)header[5] << 8) |
                 ((unsigned long)header[6] << 16) |
                 ((unsigned long)header[7] << 24);
        if (length == 0 || length > legacy_limit)
            return 0;
        *random_value = 0;
        *is_current_format = 0;
    } else if (memcmp(header, NVRAM_PROFILE_HDR2, 4) == 0) {
        length = (unsigned long)header[4] |
                 ((unsigned long)header[5] << 8) |
                 ((unsigned long)header[6] << 16);
        if (length == 0 || length > current_limit)
            return 0;
        *random_value = header[7];
        *is_current_format = 1;
    } else {
        return 0;
    }

    *file_length = length;
    return 1;
}

static void nvram_profile_decode_payload(unsigned char *buffer,
                                         unsigned long length,
                                         unsigned char random_value)
{
    unsigned long i;

    if (!buffer)
        return;

    for (i = 0; i < length; ++i) {
        if (buffer[i] > 0xfc)
            buffer[i] = '\0';
        else
            buffer[i] = (unsigned char)(0xffU + random_value - buffer[i]);
    }
}

/* Validate the complete NUL-delimited name=value list before NVRAM is cleared. */
static int nvram_profile_payload_valid(const unsigned char *buffer,
                                       unsigned long length,
                                       unsigned long max_name_length,
                                       unsigned long max_value_length)
{
    unsigned long position = 0;

    if (!buffer && length != 0)
        return 0;

    while (position < length) {
        unsigned long start, end, equal;

        if (buffer[position] == '\0') {
            /* Padding after the first empty record must decode entirely to NUL. */
            while (position < length) {
                if (buffer[position] != '\0')
                    return 0;
                ++position;
            }
            return 1;
        }

        start = position;
        while (position < length && buffer[position] != '\0')
            ++position;
        if (position == length)
            return 0;
        end = position;

        equal = start;
        while (equal < end && buffer[equal] != '=')
            ++equal;
        if (equal == start || equal == end)
            return 0;
        if (equal - start > max_name_length)
            return 0;
        if (end - equal - 1 > max_value_length)
            return 0;

        ++position;
        if (position == length)
            return 0; /* Require an empty-record terminator after the last entry. */
    }

    return 1;
}

#endif /* NVRAM_PROFILE_POLICY_H */

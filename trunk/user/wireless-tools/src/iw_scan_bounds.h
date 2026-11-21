/*
 * Bounds helpers for Wireless Extensions scan request/event parsing.
 */
#ifndef IW_SCAN_BOUNDS_H
#define IW_SCAN_BOUNDS_H

#include <stddef.h>

static inline int
iw_scan_essid_length_valid(size_t essid_len, size_t essid_capacity)
{
    return essid_len <= essid_capacity;
}

static inline int
iw_scan_event_length_valid(size_t available_len,
                           unsigned int event_len,
                           unsigned int minimum_len)
{
    return ((size_t) event_len <= available_len) &&
           (event_len >= minimum_len);
}

#endif

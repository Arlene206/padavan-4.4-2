#ifndef UDHCP_OPTION_PARSER_POLICY_H
#define UDHCP_OPTION_PARSER_POLICY_H 1

#include <stdint.h>

/* Validate the complete DHCP TLV header and payload before reading either. */
static inline int udhcp_option_tlv_size(const uint8_t *option,
		unsigned int remaining, unsigned int *total_size)
{
	unsigned int payload_length;

	if (!option || !total_size || remaining < 2U)
		return 0;

	payload_length = (unsigned int)option[1];
	if (payload_length > remaining - 2U)
		return 0;

	*total_size = 2U + payload_length;
	return 1;
}

/* RFC 2132 option-overload has exactly one data byte and uses only bits 0/1. */
static inline int udhcp_option_overload_fields(const uint8_t *option,
		unsigned int remaining, uint8_t *fields)
{
	unsigned int total_size;

	if (!fields ||
	    !udhcp_option_tlv_size(option, remaining, &total_size) ||
	    option[0] != 0x34U ||
	    option[1] != 1U ||
	    total_size != 3U ||
	    option[2] < 1U || option[2] > 3U)
		return 0;

	*fields = option[2];
	return 1;
}

/* RFC 2131 section 4.1: option-overload appears once in main options. */
static inline int udhcp_option_overload_location_valid(
		unsigned int already_seen, unsigned int in_overloaded_field)
{
	return already_seen == 0U && in_overloaded_field == 0U;
}

#endif

#ifndef MINIUPNPD_SOAP_XML_POLICY_H
#define MINIUPNPD_SOAP_XML_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

/*
 * Decode one strictly-formed UTF-8 sequence. XML 1.0 excludes U+FFFE and
 * U+FFFF; malformed, overlong, surrogate and out-of-range sequences fail.
 * On malformed input, length remains one so the caller replaces only the
 * offending byte and can safely resynchronize at the next byte.
 */
static inline int
miniupnpd_soap_xml_decode_utf8(const unsigned char *input,
                               size_t *length, unsigned long *codepoint)
{
	unsigned char first, second, third, fourth;
	unsigned long value;

	if (!input || !length || !codepoint || input[0] == '\0')
		return 0;

	*length = 1;
	first = input[0];
	if (first < 0x80U) {
		*codepoint = first;
		return 1;
	}

	if (first >= 0xc2U && first <= 0xdfU) {
		second = input[1];
		if (second == '\0' || (second & 0xc0U) != 0x80U)
			return 0;
		value = ((unsigned long)(first & 0x1fU) << 6) |
		        (unsigned long)(second & 0x3fU);
		*length = 2;
	} else if (first >= 0xe0U && first <= 0xefU) {
		second = input[1];
		if (second == '\0' || (second & 0xc0U) != 0x80U)
			return 0;
		if ((first == 0xe0U && second < 0xa0U) ||
		    (first == 0xedU && second >= 0xa0U))
			return 0;
		third = input[2];
		if (third == '\0' || (third & 0xc0U) != 0x80U)
			return 0;
		value = ((unsigned long)(first & 0x0fU) << 12) |
		        ((unsigned long)(second & 0x3fU) << 6) |
		        (unsigned long)(third & 0x3fU);
		*length = 3;
	} else if (first >= 0xf0U && first <= 0xf4U) {
		second = input[1];
		if (second == '\0' || (second & 0xc0U) != 0x80U)
			return 0;
		if ((first == 0xf0U && second < 0x90U) ||
		    (first == 0xf4U && second > 0x8fU))
			return 0;
		third = input[2];
		if (third == '\0' || (third & 0xc0U) != 0x80U)
			return 0;
		fourth = input[3];
		if (fourth == '\0' || (fourth & 0xc0U) != 0x80U)
			return 0;
		value = ((unsigned long)(first & 0x07U) << 18) |
		        ((unsigned long)(second & 0x3fU) << 12) |
		        ((unsigned long)(third & 0x3fU) << 6) |
		        (unsigned long)(fourth & 0x3fU);
		*length = 4;
	} else {
		return 0;
	}

	*codepoint = value;
	if (value == 0xfffeUL || value == 0xffffUL)
		return 0;
	return 1;
}

/* Escape untrusted XML text-node content and keep output valid XML 1.0 UTF-8. */
static inline int
miniupnpd_soap_xml_escape(char *output, size_t output_size,
                          const char *input)
{
	const unsigned char *cursor;
	size_t used = 0;

	if (!output || output_size == 0 || !input) {
		errno = EINVAL;
		return -1;
	}
	output[0] = '\0';

	for (cursor = (const unsigned char *)input; *cursor; ) {
		const char *entity = NULL;
		size_t entity_length;
		size_t sequence_length = 1;
		unsigned long codepoint = 0;
		char replacement = ' ';

		if (miniupnpd_soap_xml_decode_utf8(cursor, &sequence_length,
		                                   &codepoint)) {
			if (sequence_length > 1) {
				if (sequence_length >= output_size - used) {
					output[0] = '\0';
					errno = ENOSPC;
					return -1;
				}
				memcpy(output + used, cursor, sequence_length);
				used += sequence_length;
				cursor += sequence_length;
				continue;
			}

			replacement = (char)*cursor;
			switch (*cursor) {
			case '&':
				entity = "&amp;";
				break;
			case '<':
				entity = "&lt;";
				break;
			case '>':
				entity = "&gt;";
				break;
			case '"':
				entity = "&quot;";
				break;
			case '\'':
				entity = "&apos;";
				break;
			default:
				if ((codepoint < 0x20UL && codepoint != 0x09UL) ||
				    codepoint == 0x7fUL)
					replacement = ' ';
				break;
			}
		}

		if (entity) {
			entity_length = strlen(entity);
			if (entity_length >= output_size - used) {
				output[0] = '\0';
				errno = ENOSPC;
				return -1;
			}
			memcpy(output + used, entity, entity_length);
			used += entity_length;
		} else {
			if (used + 1 >= output_size) {
				output[0] = '\0';
				errno = ENOSPC;
				return -1;
			}
			output[used++] = replacement;
		}
		cursor += sequence_length;
	}
	output[used] = '\0';
	return 0;
}

#endif /* MINIUPNPD_SOAP_XML_POLICY_H */

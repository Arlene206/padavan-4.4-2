#ifndef WSD_XML_TEXT_POLICY_H
#define WSD_XML_TEXT_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/*
 * XML 1.0 character data is UTF-8 unless another encoding is declared.
 * Reject malformed UTF-8 and code points forbidden by the XML 1.0 Char
 * production before reflecting configuration or hostname text into SOAP.
 */
static inline int
wsd_xml_text_is_valid_xml10_utf8(const unsigned char *cursor)
{
	while (*cursor) {
		unsigned int codepoint;
		unsigned char first = cursor[0];

		if (first < 0x80) {
			codepoint = first;
			++cursor;
		} else if (first >= 0xc2 && first <= 0xdf) {
			if (cursor[1] < 0x80 || cursor[1] > 0xbf)
				goto invalid_utf8;
			codepoint = ((unsigned int)(first & 0x1f) << 6) |
				    (unsigned int)(cursor[1] & 0x3f);
			cursor += 2;
		} else if (first >= 0xe0 && first <= 0xef) {
			unsigned char second, third;

			if (!cursor[1] || !cursor[2])
				goto invalid_utf8;
			second = cursor[1];
			third = cursor[2];
			if (second < 0x80 || second > 0xbf ||
			    third < 0x80 || third > 0xbf)
				goto invalid_utf8;
			codepoint = ((unsigned int)(first & 0x0f) << 12) |
				    ((unsigned int)(second & 0x3f) << 6) |
				    (unsigned int)(third & 0x3f);
			if (codepoint < 0x800 ||
			    (codepoint >= 0xd800 && codepoint <= 0xdfff))
				goto invalid_utf8;
			cursor += 3;
		} else if (first >= 0xf0 && first <= 0xf4) {
			unsigned char second, third, fourth;

			if (!cursor[1] || !cursor[2] || !cursor[3])
				goto invalid_utf8;
			second = cursor[1];
			third = cursor[2];
			fourth = cursor[3];
			if (second < 0x80 || second > 0xbf ||
			    third < 0x80 || third > 0xbf ||
			    fourth < 0x80 || fourth > 0xbf)
				goto invalid_utf8;
			codepoint = ((unsigned int)(first & 0x07) << 18) |
				    ((unsigned int)(second & 0x3f) << 12) |
				    ((unsigned int)(third & 0x3f) << 6) |
				    (unsigned int)(fourth & 0x3f);
			if (codepoint < 0x10000 || codepoint > 0x10ffff)
				goto invalid_utf8;
			cursor += 4;
		} else {
			goto invalid_utf8;
		}

		if (!(codepoint == 0x09 || codepoint == 0x0a ||
		      codepoint == 0x0d ||
		      (codepoint >= 0x20 && codepoint <= 0xd7ff) ||
		      (codepoint >= 0xe000 && codepoint <= 0xfffd) ||
		      (codepoint >= 0x10000 && codepoint <= 0x10ffff))) {
			errno = EINVAL;
			return -1;
		}
	}

	return 0;

invalid_utf8:
	errno = EILSEQ;
	return -1;
}

/*
 * Escape untrusted text before embedding it in an XML character-data node.
 * This also rejects malformed UTF-8 and XML 1.0-forbidden code points.
 */
static inline char *
wsd_xml_escape_text(const char *input)
{
	const char *cursor;
	char *escaped;
	char *out;
	size_t input_len;
	size_t capacity;

	if (!input) {
		errno = EINVAL;
		return NULL;
	}

	input_len = strlen(input);
	if (input_len > (((size_t)-1) - 1) / 6) {
		errno = ENOMEM;
		return NULL;
	}
	if (wsd_xml_text_is_valid_xml10_utf8(
		    (const unsigned char *)input) != 0)
		return NULL;

	capacity = input_len * 6 + 1;
	escaped = malloc(capacity);
	if (!escaped)
		return NULL;
	out = escaped;
	for (cursor = input; *cursor; ++cursor) {
		const char *replacement = NULL;
		size_t replacement_len = 0;

		switch (*cursor) {
		case '&':
			replacement = "&amp;";
			replacement_len = 5;
			break;
		case '<':
			replacement = "&lt;";
			replacement_len = 4;
			break;
		case '>':
			replacement = "&gt;";
			replacement_len = 4;
			break;
		case '"':
			replacement = "&quot;";
			replacement_len = 6;
			break;
		case '\'':
			replacement = "&apos;";
			replacement_len = 6;
			break;
		default:
			*out++ = *cursor;
			break;
		}
		if (replacement) {
			memcpy(out, replacement, replacement_len);
			out += replacement_len;
		}
	}
	*out = '\0';
	return escaped;
}

#endif

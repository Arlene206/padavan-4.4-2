#ifndef WSD_XML_TEXT_POLICY_H
#define WSD_XML_TEXT_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/*
 * Escape untrusted text before embedding it in an XML character-data node.
 * XML 1.0 forbids most C0 controls, so reject those rather than reflecting
 * malformed XML into discovery responses.
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
	capacity = input_len * 6 + 1;
	for (cursor = input; *cursor; ++cursor) {
		unsigned char ch = (unsigned char)*cursor;

		if (ch < 0x20 && ch != '\t' && ch != '\n' && ch != '\r') {
			errno = EINVAL;
			return NULL;
		}
	}

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

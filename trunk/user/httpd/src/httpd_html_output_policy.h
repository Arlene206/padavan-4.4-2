#ifndef HTTPD_HTML_OUTPUT_POLICY_H
#define HTTPD_HTML_OUTPUT_POLICY_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Encode one byte for an HTML text or quoted-attribute context. */
static size_t
httpd_html_escape_byte(unsigned char input, char output[8])
{
	const char *entity = NULL;
	int length;

	switch (input) {
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
		entity = "&#39;";
		break;
	default:
		break;
	}

	if (entity) {
		length = (int)strlen(entity);
		memcpy(output, entity, (size_t)length);
		return (size_t)length;
	}

	if (input < 0x20 || input == 0x7f) {
		length = snprintf(output, 8, "&#%u;", (unsigned int)input);
		if (length < 0 || length >= 8)
			return 0;
		return (size_t)length;
	}

	/* Preserve UTF-8 bytes; escaping is applied to ASCII syntax bytes. */
	output[0] = (char)input;
	return 1;
}

#endif /* HTTPD_HTML_OUTPUT_POLICY_H */

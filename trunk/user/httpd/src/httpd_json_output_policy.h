#ifndef HTTPD_JSON_OUTPUT_POLICY_H
#define HTTPD_JSON_OUTPUT_POLICY_H

#include <stddef.h>
#include <string.h>

/*
 * Encode input for a JSON string embedded in HTML script source.
 * The caller supplies the surrounding quotes. The result is bounded to
 * six bytes and reports input consumption for UTF-8 separators.
 */
static size_t
httpd_json_escape_next(const unsigned char *input, char output[6],
		       size_t *input_consumed)
{
	static const char hex[] = "0123456789abcdef";
	unsigned char c;

	if (!input_consumed)
		return 0;
	*input_consumed = 0;
	if (!input || !output || !*input)
		return 0;

	c = input[0];
	*input_consumed = 1;

	/* Keep U+2028/U+2029 valid in older JavaScript parsers. */
	if (c == 0xe2 && input[1] == 0x80 && input[2] != '\0' &&
	    (input[2] == 0xa8 || input[2] == 0xa9)) {
		output[0] = '\\';
		output[1] = 'u';
		output[2] = '2';
		output[3] = '0';
		output[4] = '2';
		output[5] = (input[2] == 0xa8) ? '8' : '9';
		*input_consumed = 3;
		return 6;
	}

	switch (c) {
	case '"':
		output[0] = '\\';
		output[1] = '"';
		return 2;
	case '\\':
		output[0] = '\\';
		output[1] = '\\';
		return 2;
	case '<':
		memcpy(output, "\\u003c", 6);
		return 6;
	case '>':
		memcpy(output, "\\u003e", 6);
		return 6;
	case '&':
		memcpy(output, "\\u0026", 6);
		return 6;
	case '\b':
		output[0] = '\\';
		output[1] = 'b';
		return 2;
	case '\f':
		output[0] = '\\';
		output[1] = 'f';
		return 2;
	case '\n':
		output[0] = '\\';
		output[1] = 'n';
		return 2;
	case '\r':
		output[0] = '\\';
		output[1] = 'r';
		return 2;
	case '\t':
		output[0] = '\\';
		output[1] = 't';
		return 2;
	default:
		if (c < 0x20) {
			output[0] = '\\';
			output[1] = 'u';
			output[2] = '0';
			output[3] = '0';
			output[4] = hex[(c >> 4) & 0x0f];
			output[5] = hex[c & 0x0f];
			return 6;
		}
		output[0] = (char)c;
		return 1;
	}
}

#endif /* HTTPD_JSON_OUTPUT_POLICY_H */

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "httpd_json_output_policy.h"

static void
assert_escaped(const unsigned char *input, const char *expected)
{
	char output[1024];
	size_t input_len = strlen((const char *)input);
	size_t input_offset = 0;
	size_t output_len = 0;

	while (input_offset < input_len) {
		char escaped[6];
		size_t consumed = 0;
		size_t length = httpd_json_escape_next(input + input_offset,
						       escaped, &consumed);

		assert(consumed > 0);
		assert(consumed <= input_len - input_offset);
		assert(length > 0 && length <= sizeof(escaped));
		assert(output_len + length < sizeof(output));
		memcpy(output + output_len, escaped, length);
		input_offset += consumed;
		output_len += length;
	}

	output[output_len] = '\0';
	assert(strcmp(output, expected) == 0);
}

int
main(void)
{
	static const unsigned char quote[] = { '"', 0 };
	static const unsigned char backslash[] = { '\\', 0 };
	static const unsigned char markup[] = "</script><b>&";
	static const unsigned char wireless_ssid[] = "\"<script>";
	static const unsigned char controls[] = {
		'\b', '\f', '\n', '\r', '\t', 0x01, 0x1f, 0
	};
	static const unsigned char unicode_separators[] = {
		0xe2, 0x80, 0xa8, 0xe2, 0x80, 0xa9, 0
	};
	static const unsigned char utf8_text[] = {
		0xc3, 0xa9, ' ', 0xe4, 0xb8, 0xad, 0
	};
	static const unsigned char truncated_separator[] = {
		0xe2, 0x80, 0
	};
	char output[6];
	size_t consumed = 99;

	assert_escaped(quote, "\\\"");
	assert_escaped(backslash, "\\\\");
	assert_escaped(markup, "\\u003c/script\\u003e\\u003cb\\u003e\\u0026");
	assert_escaped(wireless_ssid, "\\\"\\u003cscript\\u003e");
	assert_escaped(controls, "\\b\\f\\n\\r\\t\\u0001\\u001f");
	assert_escaped(unicode_separators, "\\u2028\\u2029");
	assert_escaped(utf8_text, "\xc3\xa9 \xe4\xb8\xad");
	assert_escaped(truncated_separator, "\xe2\x80");

	assert(httpd_json_escape_next(NULL, output, &consumed) == 0);
	assert(consumed == 0);
	consumed = 99;
	assert(httpd_json_escape_next((const unsigned char *)"", output,
				      &consumed) == 0);
	assert(consumed == 0);

	return 0;
}

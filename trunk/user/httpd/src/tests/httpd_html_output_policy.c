#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "httpd_html_output_policy.h"

static void
assert_escaped(const unsigned char *input, const char *expected)
{
	char output[256];
	size_t input_offset = 0;
	size_t output_length = 0;
	size_t input_length = strlen((const char *)input);

	while (input_offset < input_length) {
		char encoded[8];
		size_t length = httpd_html_escape_byte(input[input_offset], encoded);

		assert(length > 0 && length < sizeof(encoded));
		assert(output_length + length < sizeof(output));
		memcpy(output + output_length, encoded, length);
		output_length += length;
		++input_offset;
	}

	output[output_length] = '\0';
	assert(strcmp(output, expected) == 0);
}

int
main(void)
{
	static const unsigned char syntax[] = "\"'&<>";
	static const unsigned char textarea_payload[] =
		"</textarea><script>alert(1)</script>";
	static const unsigned char controls[] = {
		0x01, '\b', '\n', '\r', '\t', 0x7f, 0
	};
	static const unsigned char utf8[] = {
		0xc3, 0xa9, ' ', 0xe4, 0xb8, 0xad, 0
	};

	assert_escaped((const unsigned char *)"plain text", "plain text");
	assert_escaped(syntax, "&quot;&#39;&amp;&lt;&gt;");
	assert_escaped(textarea_payload,
		       "&lt;/textarea&gt;&lt;script&gt;alert(1)&lt;/script&gt;");
	assert_escaped(controls, "&#1;&#8;&#10;&#13;&#9;&#127;");
	assert_escaped(utf8, "\xc3\xa9 \xe4\xb8\xad");
	return 0;
}

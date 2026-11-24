#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "wsd_xml_text_policy.h"

static void
test_plain_text_is_preserved(void)
{
	char *result = wsd_xml_escape_text("urn:uuid:12345678-1234-1234-1234-123456789abc");

	assert(result != NULL);
	assert(strcmp(result, "urn:uuid:12345678-1234-1234-1234-123456789abc") == 0);
	free(result);
}

static void
test_markup_characters_are_escaped(void)
{
	char *result = wsd_xml_escape_text("<&>\"'");

	assert(result != NULL);
	assert(strcmp(result, "&lt;&amp;&gt;&quot;&apos;") == 0);
	free(result);
}

static void
test_configured_metadata_text_is_escaped(void)
{
	char *vendor = wsd_xml_escape_text("Acme & Partners <Networks>");
	char *model_url = wsd_xml_escape_text("https://router.test/?a=1&b=2");
	char *workgroup = wsd_xml_escape_text("Office & Sales");

	assert(vendor != NULL);
	assert(strcmp(vendor, "Acme &amp; Partners &lt;Networks&gt;") == 0);
	assert(model_url != NULL);
	assert(strcmp(model_url, "https://router.test/?a=1&amp;b=2") == 0);
	assert(workgroup != NULL);
	assert(strcmp(workgroup, "Office &amp; Sales") == 0);
	free(vendor);
	free(model_url);
	free(workgroup);
}

static void
test_xml_whitespace_is_preserved(void)
{
	char *result = wsd_xml_escape_text("line1\tline2\nline3\r");

	assert(result != NULL);
	assert(strcmp(result, "line1\tline2\nline3\r") == 0);
	free(result);
}

static void
test_invalid_controls_and_null_are_rejected(void)
{
	const char invalid[] = {'a', 1, 'b', '\0'};

	errno = 0;
	assert(wsd_xml_escape_text(invalid) == NULL);
	assert(errno == EINVAL);
	errno = 0;
	assert(wsd_xml_escape_text(NULL) == NULL);
	assert(errno == EINVAL);
}

static void
test_valid_utf8_text_is_preserved(void)
{
	const char input[] = "R\xc3\xb6uter \xe6\x97\xa5\xe6\x9c\xac \xf0\x9f\x9a\x80";
	char *result = wsd_xml_escape_text(input);

	assert(result != NULL);
	assert(strcmp(result, input) == 0);
	free(result);
}

static void
test_malformed_utf8_is_rejected(void)
{
	static const char overlong[] = { (char)0xc0, (char)0x80, '\0' };
	static const char orphan[] = { (char)0x80, '\0' };
	static const char truncated_three[] = { (char)0xe2, (char)0x82, '\0' };
	static const char bad_continuation[] = { (char)0xe2, 'x', (char)0xa1, '\0' };
	static const char surrogate[] = { (char)0xed, (char)0xa0, (char)0x80, '\0' };
	static const char too_large[] = {
		(char)0xf4, (char)0x90, (char)0x80, (char)0x80, '\0'
	};
	const char *invalid[] = {
		overlong, orphan, truncated_three, bad_continuation, surrogate,
		too_large
	};
	size_t i;

	for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
		errno = 0;
		assert(wsd_xml_escape_text(invalid[i]) == NULL);
		assert(errno == EILSEQ);
	}
}

static void
test_xml_forbidden_codepoints_are_rejected(void)
{
	static const char forbidden_control[] = { 'x', 0x0b, 'y', '\0' };
	static const char unicode_fffe[] = {
		(char)0xef, (char)0xbf, (char)0xbe, '\0'
	};

	errno = 0;
	assert(wsd_xml_escape_text(forbidden_control) == NULL);
	assert(errno == EINVAL);
	errno = 0;
	assert(wsd_xml_escape_text(unicode_fffe) == NULL);
	assert(errno == EINVAL);
}

static void
test_empty_and_large_text(void)
{
	char *empty = wsd_xml_escape_text("");
	char *input;
	char *result;
	size_t i;

	assert(empty != NULL && empty[0] == '\0');
	free(empty);
	input = malloc(4097);
	assert(input != NULL);
	for (i = 0; i < 4096; ++i)
		input[i] = '&';
	input[4096] = '\0';
	result = wsd_xml_escape_text(input);
	assert(result != NULL);
	for (i = 0; i < 4096; ++i)
		assert(memcmp(result + i * 5, "&amp;", 5) == 0);
	assert(result[4096 * 5] == '\0');
	free(result);
	free(input);
}

int
main(void)
{
	test_plain_text_is_preserved();
	test_markup_characters_are_escaped();
	test_configured_metadata_text_is_escaped();
	test_xml_whitespace_is_preserved();
	test_invalid_controls_and_null_are_rejected();
	test_valid_utf8_text_is_preserved();
	test_malformed_utf8_is_rejected();
	test_xml_forbidden_codepoints_are_rejected();
	test_empty_and_large_text();
	return 0;
}

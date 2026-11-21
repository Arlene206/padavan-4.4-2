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
	test_xml_whitespace_is_preserved();
	test_invalid_controls_and_null_are_rejected();
	test_empty_and_large_text();
	return 0;
}

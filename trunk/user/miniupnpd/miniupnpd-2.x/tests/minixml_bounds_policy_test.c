#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minixml.h"
#include "minixml_bounds_policy.h"

struct parser_state {
	int data_calls;
	int data_length;
	char data[32];
};

static void
record_data(void *context, const char *data, int length)
{
	struct parser_state *state = (struct parser_state *)context;
	assert(state != NULL);
	assert(length >= 0);
	assert((size_t)length < sizeof(state->data));
	state->data_calls++;
	state->data_length = length;
	memcpy(state->data, data, (size_t)length);
	state->data[length] = '\0';
}

static void
parse_text(const char *input, struct parser_state *state)
{
	struct xmlparser parser;
	char *buffer;
	size_t length = strlen(input);
	assert(length > 0U && length <= (size_t)INT_MAX);
	buffer = (char *)malloc(length);
	assert(buffer != NULL);
	memcpy(buffer, input, length);
	memset(&parser, 0, sizeof(parser));
	parser.xmlstart = buffer;
	parser.xmlsize = (int)length;
	parser.data = state;
	parser.datafunc = record_data;
	parsexml(&parser);
	free(buffer);
}

static void
test_sequence_bounds(void)
{
	static const char bytes[] = "<![CDATA[";
	const char *end = bytes + sizeof(bytes) - 1U;
	assert(miniupnpd_xml_sequence_equal(bytes, end, "<![CDATA[", 9U));
	assert(miniupnpd_xml_sequence_equal(bytes + 1, end, "![CDATA[", 8U));
	assert(!miniupnpd_xml_sequence_equal(bytes, bytes + 8, "<![CDATA[", 9U));
	assert(!miniupnpd_xml_sequence_equal(bytes + 8, end, "x", 1U));
	assert(!miniupnpd_xml_sequence_equal(bytes, end, "", 0U));
	assert(!miniupnpd_xml_sequence_equal(NULL, end, "<![CDATA[", 9U));
	assert(!miniupnpd_xml_sequence_equal(bytes, NULL, "<![CDATA[", 9U));
	assert(!miniupnpd_xml_sequence_equal(bytes, end, NULL, 9U));
}

static void
test_truncated_comment_and_cdata_openers(void)
{
	struct parser_state state;
	memset(&state, 0, sizeof(state));
	parse_text("<!--", &state);
	parse_text("<a><!--", &state);
	parse_text("<a><![C", &state);
	parse_text("<a><![CDATA", &state);
	assert(state.data_calls == 0);
}

static void
test_truncated_cdata_content_and_terminators(void)
{
	struct parser_state state;
	memset(&state, 0, sizeof(state));
	parse_text("<a><![CDATA[", &state);
	parse_text("<a><![CDATA[x", &state);
	parse_text("<a><![CDATA[x]]", &state);
	assert(state.data_calls == 0);
	/* A complete terminator at end-of-buffer must not dereference end. */
	parse_text("<a><![CDATA[x]]>", &state);
	assert(state.data_calls == 1);
	assert(state.data_length == 1);
	assert(strcmp(state.data, "x") == 0);
}

static void
test_valid_cdata_and_comment(void)
{
	struct parser_state state;
	memset(&state, 0, sizeof(state));
	parse_text("<!-- comment --><a><![CDATA[hello]]></a>", &state);
	assert(state.data_calls == 1);
	assert(state.data_length == 5);
	assert(strcmp(state.data, "hello") == 0);
}

static void
test_parsexml_rejects_invalid_inputs(void)
{
	struct xmlparser parser;
	memset(&parser, 0, sizeof(parser));
	parsexml(NULL);
	parser.xmlstart = NULL;
	parser.xmlsize = 8;
	parsexml(&parser);
	parser.xmlstart = "x";
	parser.xmlsize = 0;
	parsexml(&parser);
	parser.xmlsize = -1;
	parsexml(&parser);
}

int
main(void)
{
	test_sequence_bounds();
	test_truncated_comment_and_cdata_openers();
	test_truncated_cdata_content_and_terminators();
	test_valid_cdata_and_comment();
	test_parsexml_rejects_invalid_inputs();
	puts("MiniUPnP bounded XML parser sequence regression: PASS");
	return 0;
}

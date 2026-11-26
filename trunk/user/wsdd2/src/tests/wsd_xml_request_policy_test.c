#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "wsd_xml_request_policy.h"

static const char *action_tags[][2] = {
	{ "<wsa:Action>", "</wsa:Action>" },
	{ "<wsa:Action SOAP-ENV:mustUnderstand=\"true\">",
	  "</wsa:Action>" }
};

static const char *message_id_tags[][2] = {
	{ "<wsa:MessageID>", "</wsa:MessageID>" },
	{ "<wsa:MessageID SOAP-ENV:mustUnderstand=\"true\">",
	  "</wsa:MessageID>" }
};

static const char *
find_action(const char *xml, size_t length, size_t *value_length)
{
	return wsd_xml_find_simple_text(xml, length, action_tags, 2,
	                               value_length);
}

static void
test_extracts_real_text_elements(void)
{
	static const char xml[] =
		"<?xml version=\"1.0\"?>"
		"<s:Envelope><s:Header>"
		"<wsa:Action> urn:schemas-xmlsoap-org:ws:2005:04:discovery:Probe </wsa:Action>"
		"<wsa:MessageID> urn:uuid:1234 </wsa:MessageID>"
		"</s:Header></s:Envelope>";
	size_t action_length = 0, id_length = 0;
	const char *action = find_action(xml, sizeof(xml) - 1, &action_length);
	const char *id = wsd_xml_find_simple_text(
		xml, sizeof(xml) - 1, message_id_tags, 2, &id_length);

	assert(action != NULL && action_length == strlen(
		"urn:schemas-xmlsoap-org:ws:2005:04:discovery:Probe"));
	assert(memcmp(action, "urn:schemas-xmlsoap-org:ws:2005:04:discovery:Probe",
	              action_length) == 0);
	assert(id != NULL && id_length == strlen("urn:uuid:1234"));
	assert(memcmp(id, "urn:uuid:1234", id_length) == 0);
}

static void
test_ignores_tag_lookalikes_in_comments_and_cdata(void)
{
	static const char comment_only[] =
		"<Envelope><!-- <wsa:Action>Probe</wsa:Action> --></Envelope>";
	static const char cdata_only[] =
		"<Envelope><![CDATA[<wsa:Action>Probe</wsa:Action>]]></Envelope>";
	static const char mixed[] =
		"<!-- <wsa:Action>Probe</wsa:Action> -->"
		"<wsa:Action>Resolve</wsa:Action>";
	size_t length = 99;
	const char *value;

	assert(find_action(comment_only, sizeof(comment_only) - 1, &length) == NULL);
	assert(length == 99);
	assert(find_action(cdata_only, sizeof(cdata_only) - 1, &length) == NULL);
	value = find_action(mixed, sizeof(mixed) - 1, &length);
	assert(value != NULL && length == strlen("Resolve"));
	assert(memcmp(value, "Resolve", length) == 0);
}

static void
test_ignores_tag_lookalikes_in_quoted_attributes_and_processing_instructions(void)
{
	static const char attribute_only[] =
		"<Envelope note=\"<wsa:Action>Probe</wsa:Action>\"></Envelope>";
	static const char pi_only[] =
		"<?router <wsa:Action>Probe</wsa:Action> ?>"
		"<Envelope/>";
	size_t length = 0;

	assert(find_action(attribute_only, sizeof(attribute_only) - 1,
	                   &length) == NULL);
	assert(find_action(pi_only, sizeof(pi_only) - 1, &length) == NULL);
}

static void
test_rejects_duplicate_empty_and_nested_elements(void)
{
	static const char duplicate[] =
		"<wsa:Action>Probe</wsa:Action>"
		"<wsa:Action>Resolve</wsa:Action>";
	static const char duplicate_variant[] =
		"<wsa:Action>Probe</wsa:Action>"
		"<wsa:Action SOAP-ENV:mustUnderstand=\"true\">Resolve</wsa:Action>";
	static const char empty[] = "<wsa:Action> \t\r\n </wsa:Action>";
	static const char nested[] =
		"<wsa:Action><x>Probe</x></wsa:Action>";
	size_t length = 0;

	assert(find_action(duplicate, sizeof(duplicate) - 1, &length) == NULL);
	assert(find_action(duplicate_variant, sizeof(duplicate_variant) - 1,
	                   &length) == NULL);
	assert(find_action(empty, sizeof(empty) - 1, &length) != NULL);
	assert(length == 0);
	assert(find_action(nested, sizeof(nested) - 1, &length) == NULL);
}

static void
test_bounded_input_and_malformed_markup(void)
{
	static const char with_suffix[] =
		"<wsa:Action>Probe</wsa:Action>"
		"<wsa:Action>OutOfRange</wsa:Action>";
	static const char unterminated_comment[] =
		"<!-- <wsa:Action>Probe</wsa:Action>";
	static const char unterminated_tag[] = "<Envelope";
	static const char declaration[] =
		"<!DOCTYPE Envelope [ <!ENTITY x \"x\"> ]>"
		"<wsa:Action>Probe</wsa:Action>";
	static const char embedded_nul[] = {
		'<','w','s','a',':','A','c','t','i','o','n','>',
		'P','r','o','b','e','<','/','w','s','a',':','A','c','t','i','o','n','>',
		'\0','<','w','s','a',':','A','c','t','i','o','n','>','X'
	};
	static const char bounded[] = "ab";
	size_t length = 0;
	const char *value;

	value = find_action(with_suffix,
	                    sizeof("<wsa:Action>Probe</wsa:Action>") - 1,
	                    &length);
	assert(value != NULL && length == strlen("Probe"));
	assert(memcmp(value, "Probe", length) == 0);
	assert(find_action(unterminated_comment,
	                   sizeof(unterminated_comment) - 1, &length) == NULL);
	assert(find_action(unterminated_tag,
	                   sizeof(unterminated_tag) - 1, &length) == NULL);
	assert(find_action(declaration, sizeof(declaration) - 1, &length) == NULL);
	assert(find_action(embedded_nul, sizeof(embedded_nul), &length) == NULL);
	assert(wsd_xml_find_simple_text(NULL, 0, action_tags, 2, &length) == NULL);
	assert(wsd_xml_find_sequence(bounded, bounded + 2, "abc") == NULL);
}

int
main(void)
{
	test_extracts_real_text_elements();
	test_ignores_tag_lookalikes_in_comments_and_cdata();
	test_ignores_tag_lookalikes_in_quoted_attributes_and_processing_instructions();
	test_rejects_duplicate_empty_and_nested_elements();
	test_bounded_input_and_malformed_markup();
	return 0;
}

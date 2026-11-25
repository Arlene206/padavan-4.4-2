#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "upnpsoap_xml_policy.h"

static void
test_markup_escaping(void)
{
	char escaped[128];

	assert(miniupnpd_soap_xml_escape(escaped, sizeof(escaped),
	                                  "A&B <tag> \"quoted\" 'single'") == 0);
	assert(strcmp(escaped,
	              "A&amp;B &lt;tag&gt; &quot;quoted&quot; &apos;single&apos;") == 0);
}

static void
test_control_character_sanitization(void)
{
	char escaped[64];

	assert(miniupnpd_soap_xml_escape(escaped, sizeof(escaped),
	                                  "one\ntwo\rthree\001four\tfive") == 0);
	assert(strcmp(escaped, "one two three four\tfive") == 0);
}

static void
test_empty_and_invalid_inputs(void)
{
	char escaped[8] = "stale";

	assert(miniupnpd_soap_xml_escape(escaped, sizeof(escaped), "") == 0);
	assert(strcmp(escaped, "") == 0);

	errno = 0;
	assert(miniupnpd_soap_xml_escape(escaped, 5, "&") == -1);
	assert(errno == ENOSPC);
	assert(escaped[0] == '\0');

	errno = 0;
	assert(miniupnpd_soap_xml_escape(escaped, 0, "x") == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(miniupnpd_soap_xml_escape(NULL, sizeof(escaped), "x") == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(miniupnpd_soap_xml_escape(escaped, sizeof(escaped), NULL) == -1);
	assert(errno == EINVAL);
}

static void
test_utf8_well_formed_sequences_are_preserved(void)
{
	char escaped[64];
	const char input[] = {
		'A', (char)0xc2, (char)0xa2, (char)0xe2, (char)0x82, (char)0xac,
		(char)0xf0, (char)0x9f, (char)0x8c, (char)0x90, '\0'
	};

	assert(miniupnpd_soap_xml_escape(escaped, sizeof(escaped), input) == 0);
	assert(memcmp(escaped, input, sizeof(input)) == 0);
}

static void
test_malformed_utf8_is_replaced_without_leaking_invalid_bytes(void)
{
	char escaped[64];
	const char input[] = {
		'a', (char)0x80, 'b', (char)0xc0, (char)0xaf, 'c',
		(char)0xe2, (char)0x82, '\0'
	};

	assert(miniupnpd_soap_xml_escape(escaped, sizeof(escaped), input) == 0);
	assert(strcmp(escaped, "a b  c  ") == 0);
}

static void
test_surrogates_out_of_range_and_xml_noncharacters_are_replaced(void)
{
	char escaped[64];
	const char invalid[] = {
		(char)0xed, (char)0xa0, (char)0x80, ' ',
		(char)0xf4, (char)0x90, (char)0x80, (char)0x80, ' ',
		(char)0xef, (char)0xbf, (char)0xbe, ' ',
		(char)0xef, (char)0xbf, (char)0xbf, '\0'
	};
	const char expected[] = "            ";

	assert(miniupnpd_soap_xml_escape(escaped, sizeof(escaped), invalid) == 0);
	assert(strcmp(escaped, expected) == 0);
}

static void
test_multibyte_sequence_never_partially_fills_output(void)
{
	char escaped[3] = "x";
	const char input[] = {(char)0xe2, (char)0x82, (char)0xac, '\0'};

	errno = 0;
	assert(miniupnpd_soap_xml_escape(escaped, sizeof(escaped), input) == -1);
	assert(errno == ENOSPC);
	assert(escaped[0] == '\0');
}

int
main(void)
{
	test_markup_escaping();
	test_control_character_sanitization();
	test_utf8_well_formed_sequences_are_preserved();
	test_malformed_utf8_is_replaced_without_leaking_invalid_bytes();
	test_surrogates_out_of_range_and_xml_noncharacters_are_replaced();
	test_multibyte_sequence_never_partially_fills_output();
	test_empty_and_invalid_inputs();
	puts("MiniUPnP SOAP XML text escaping policy: PASS");
	return 0;
}

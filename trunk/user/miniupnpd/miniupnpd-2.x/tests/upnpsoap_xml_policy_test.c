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

int
main(void)
{
	test_markup_escaping();
	test_control_character_sanitization();
	test_empty_and_invalid_inputs();
	puts("MiniUPnP SOAP XML text escaping policy: PASS");
	return 0;
}

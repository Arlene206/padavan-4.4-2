#ifndef MINIUPNPD_SOAP_XML_POLICY_H
#define MINIUPNPD_SOAP_XML_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

/* Escape untrusted XML text-node content. Never emit a partial entity. */
static inline int
miniupnpd_soap_xml_escape(char *output, size_t output_size,
                          const char *input)
{
	const unsigned char *cursor;
	size_t used = 0;

	if (!output || output_size == 0 || !input) {
		errno = EINVAL;
		return -1;
	}
	output[0] = '\0';

	for (cursor = (const unsigned char *)input; *cursor; ++cursor) {
		const char *entity = NULL;
		size_t entity_length;
		char replacement = (char)*cursor;

		switch (*cursor) {
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
			entity = "&apos;";
			break;
		default:
			if ((*cursor < 0x20 && *cursor != '\t') ||
			    *cursor == 0x7f)
				replacement = ' ';
			break;
		}

		if (entity) {
			entity_length = strlen(entity);
			if (entity_length >= output_size - used) {
				output[0] = '\0';
				errno = ENOSPC;
				return -1;
			}
			memcpy(output + used, entity, entity_length);
			used += entity_length;
		} else {
			if (used + 1 >= output_size) {
				output[0] = '\0';
				errno = ENOSPC;
				return -1;
			}
			output[used++] = replacement;
		}
	}
	output[used] = '\0';
	return 0;
}

#endif /* MINIUPNPD_SOAP_XML_POLICY_H */

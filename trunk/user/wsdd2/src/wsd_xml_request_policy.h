#ifndef WSD_XML_REQUEST_POLICY_H
#define WSD_XML_REQUEST_POLICY_H

#include <stddef.h>
#include <string.h>

/* Find a bounded byte sequence without relying on a trailing NUL. */
static inline const char *
wsd_xml_find_sequence(const char *start, const char *end, const char *needle)
{
	size_t needle_length = strlen(needle);

	if (needle_length > (size_t)(end - start))
		return NULL;
	while ((size_t)(end - start) >= needle_length) {
		if (memcmp(start, needle, needle_length) == 0)
			return start;
		++start;
	}
	return NULL;
}

static inline int
wsd_xml_has_prefix(const char *start, const char *end, const char *prefix)
{
	size_t prefix_length = strlen(prefix);

	return prefix_length <= (size_t)(end - start) &&
	       memcmp(start, prefix, prefix_length) == 0;
}

static inline const char *
wsd_xml_skip_delimited(const char *start, const char *end,
                       const char *terminator)
{
	const char *found = wsd_xml_find_sequence(start, end, terminator);

	if (!found)
		return NULL;
	return found + strlen(terminator);
}

/*
 * Extract one flat XML text element without confusing comment, CDATA,
 * processing-instruction, or quoted-attribute contents for real elements.
 * Declarations/DTDs and duplicate target elements are rejected. The parser
 * intentionally supports only the exact tag variants used by wsdd2.
 */
static inline const char *
wsd_xml_find_simple_text(const char *xml, size_t length,
                         const char *tags[][2], size_t tag_count,
                         size_t *value_length)
{
	const char *cursor, *end;
	const char *found_value = NULL;
	size_t found_length = 0;
	size_t matches = 0;

	if (!xml || !tags || !value_length || length == 0 ||
	    memchr(xml, '\0', length) != NULL)
		return NULL;

	cursor = xml;
	end = xml + length;
	while (cursor < end) {
		const char *markup = (const char *)memchr(
			cursor, '<', (size_t)(end - cursor));
		size_t i;
		int matched = 0;

		if (!markup)
			break;

		if (wsd_xml_has_prefix(markup, end, "<!--")) {
			cursor = wsd_xml_skip_delimited(markup + 4, end, "-->");
			if (!cursor)
				return NULL;
			continue;
		}
		if (wsd_xml_has_prefix(markup, end, "<![CDATA[")) {
			cursor = wsd_xml_skip_delimited(markup + 9, end, "]]>");
			if (!cursor)
				return NULL;
			continue;
		}
		if (wsd_xml_has_prefix(markup, end, "<?")) {
			cursor = wsd_xml_skip_delimited(markup + 2, end, "?>");
			if (!cursor)
				return NULL;
			continue;
		}
		/* Reject DTDs and declarations rather than scanning their contents. */
		if (wsd_xml_has_prefix(markup, end, "<!"))
			return NULL;

		for (i = 0; i < tag_count; ++i) {
			size_t open_length = strlen(tags[i][0]);
			size_t close_length = strlen(tags[i][1]);
			const char *value_start;
			const char *close;
			const char *nested;
			const char *trim_start;
			const char *trim_end;

			if (open_length > (size_t)(end - markup) ||
			    memcmp(markup, tags[i][0], open_length) != 0)
				continue;

			value_start = markup + open_length;
			close = wsd_xml_find_sequence(
				value_start, end, tags[i][1]);
			if (!close)
				return NULL;
			nested = (const char *)memchr(
				value_start, '<', (size_t)(close - value_start));
			if (nested)
				return NULL;

			trim_start = value_start;
			trim_end = close;
			while (trim_start < trim_end &&
			       (*trim_start == ' ' || *trim_start == '\t' ||
			        *trim_start == '\r' || *trim_start == '\n'))
				++trim_start;
			while (trim_end > trim_start &&
			       (trim_end[-1] == ' ' || trim_end[-1] == '\t' ||
			        trim_end[-1] == '\r' || trim_end[-1] == '\n'))
				--trim_end;

			if (++matches != 1)
				return NULL;
			found_value = trim_start;
			found_length = (size_t)(trim_end - trim_start);
			cursor = close + close_length;
			matched = 1;
			break;
		}
		if (matched)
			continue;

		/*
		 * Skip an ordinary start/end tag. A '>' inside a quoted
		 * attribute is data, not the end of this markup token.
		 */
		{
			const char *p = markup + 1;
			char quote = '\0';
			while (p < end) {
				if (quote) {
					if (*p == quote)
						quote = '\0';
				} else if (*p == '"' || *p == '\'') {
					quote = *p;
				} else if (*p == '>') {
					break;
				}
				++p;
			}
			if (p == end || quote)
				return NULL;
			cursor = p + 1;
		}
	}

	if (matches != 1)
		return NULL;
	*value_length = found_length;
	return found_value;
}

#endif /* WSD_XML_REQUEST_POLICY_H */

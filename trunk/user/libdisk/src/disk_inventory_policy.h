#ifndef PADAVAN_DISK_INVENTORY_POLICY_H
#define PADAVAN_DISK_INVENTORY_POLICY_H
#include <limits.h>
#include <stddef.h>
#include <string.h>
static int
disk_inventory_line_is_complete(const char *line)
{
	size_t length;
	if (!line) return 0;
	length = strlen(line);
	return length > 0 && line[length - 1] == '\n' &&
	       memchr(line, '\n', length) == line + length - 1;
}
static int
disk_inventory_line_is_blank(const char *line)
{
	const char *p;
	if (!disk_inventory_line_is_complete(line)) return 0;
	for (p = line; *p != '\n'; p++)
		if (*p != ' ' && *p != '\t' && *p != '\r') return 0;
	return p[1] == '\0';
}
static int
disk_inventory_next_token(const char **cursor, char *token, size_t token_size)
{
	const char *start, *p;
	size_t length;
	if (!cursor || !*cursor || !token || token_size == 0) return -1;
	p = *cursor;
	while (*p == ' ' || *p == '\t' || *p == '\r') p++;
	if (*p == '\0' || *p == '\n') { *cursor = p; return 0; }
	start = p;
	while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') p++;
	length = (size_t)(p - start);
	if (length == 0 || length >= token_size) return -1;
	memcpy(token, start, length);
	token[length] = '\0';
	*cursor = p;
	return 1;
}
static int
disk_inventory_parse_decimal(const char *token, unsigned long long limit,
			     unsigned long long *value)
{
	const unsigned char *p;
	unsigned long long result = 0;
	if (!token || !*token || !value) return -1;

	/* Check the 64-bit ceiling before multiply/add. A constant cutoff
	 * avoids a variable-width division and MIPS libgcc __udivdi3 import. */
	for (p = (const unsigned char *)token; *p; p++) {
		unsigned int digit;
		if (*p < '0' || *p > '9') return -1;
		digit = (unsigned int)(*p - '0');
		if (result > 1844674407370955161ULL ||
		    (result == 1844674407370955161ULL && digit > 5U))
			return -1;
		result = result * 10ULL + digit;
		if (result > limit) return -1;
	}
	*value = result;
	return 0;
}
static int
disk_inventory_parse_header_line(const char *line)
{
	const char *cursor;
	char token[16], extra[2];
	static const char *const expected[] = { "major", "minor", "#blocks", "name" };
	size_t i;
	if (!disk_inventory_line_is_complete(line)) return -1;
	cursor = line;
	for (i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
		if (disk_inventory_next_token(&cursor, token, sizeof(token)) != 1 ||
		    strcmp(token, expected[i]) != 0) return -1;
	return disk_inventory_next_token(&cursor, extra, sizeof(extra)) == 0 ? 0 : -1;
}
static int
disk_inventory_parse_partition_line(const char *line, unsigned int *major,
				     unsigned int *minor, unsigned long long *blocks,
				     char *device, size_t device_size)
{
	const char *cursor;
	char major_token[16], minor_token[16], blocks_token[32], extra[2];
	unsigned long long parsed_major, parsed_minor, parsed_blocks;
	int status;
	if (!major || !minor || !blocks || !device || device_size == 0 ||
	    !disk_inventory_line_is_complete(line)) return -1;
	device[0] = '\0';
	cursor = line;
	status = disk_inventory_next_token(&cursor, major_token, sizeof(major_token));
	if (status == 0) return 0;
	if (status != 1 ||
	    disk_inventory_parse_decimal(major_token, UINT_MAX, &parsed_major) != 0) return -1;
	if (disk_inventory_next_token(&cursor, minor_token, sizeof(minor_token)) != 1 ||
	    disk_inventory_parse_decimal(minor_token, UINT_MAX, &parsed_minor) != 0) return -1;
	if (disk_inventory_next_token(&cursor, blocks_token, sizeof(blocks_token)) != 1 ||
	    disk_inventory_parse_decimal(blocks_token, ULLONG_MAX, &parsed_blocks) != 0) return -1;
	if (disk_inventory_next_token(&cursor, device, device_size) != 1) {
		device[0] = '\0'; return -1;
	}
	if (disk_inventory_next_token(&cursor, extra, sizeof(extra)) != 0) {
		device[0] = '\0'; return -1;
	}
	*major = (unsigned int)parsed_major;
	*minor = (unsigned int)parsed_minor;
	*blocks = parsed_blocks;
	return 1;
}
#endif

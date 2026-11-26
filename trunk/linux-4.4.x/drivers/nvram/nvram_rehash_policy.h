#ifndef NVRAM_REHASH_POLICY_H
#define NVRAM_REHASH_POLICY_H

#ifndef __KERNEL__
#include <stddef.h>
#include <string.h>
#else
#include <linux/string.h>
#endif

#define NVRAM_REHASH_INVALID (-1)
#define NVRAM_REHASH_INIT_NOMEM (-5)
#define NVRAM_REHASH_INIT_INVALID (-6)

typedef int (*nvram_rehash_insert_fn)(void *context,
				      const char *name, const char *value);
typedef void (*nvram_rehash_rollback_fn)(void *context);

/*
 * Parse the CRC-validated flash data span. Every record must be a bounded
 * "name=value\\0" entry and the remaining bytes must be NUL padding.
 * A failed insertion or malformed later record rolls back all earlier
 * insertions so callers never expose a partial NVRAM generation.
 */
static inline int
nvram_rehash_records(char *data, size_t data_length,
		     nvram_rehash_insert_fn insert_fn,
		     nvram_rehash_rollback_fn rollback_fn, void *context)
{
	size_t offset = 0;

	if (!data || data_length < 2 || !insert_fn || !rollback_fn)
		return NVRAM_REHASH_INVALID;

	while (offset < data_length) {
		size_t remaining, record_length, index;
		char *equals;
		int status;

		if (data[offset] == '\0') {
			for (index = offset; index < data_length; ++index) {
				if (data[index] != '\0')
					goto invalid;
			}
			return 0;
		}

		remaining = data_length - offset;
		record_length = strnlen(data + offset, remaining);
		if (record_length >= remaining)
			goto invalid;

		equals = memchr(data + offset, '=', record_length);
		if (!equals || equals == data + offset)
			goto invalid;

		*equals = '\0';
		status = insert_fn(context, data + offset, equals + 1);
		*equals = '=';
		if (status != 0) {
			rollback_fn(context);
			return status < 0 ? status : NVRAM_REHASH_INVALID;
		}

		offset += record_length + 1;
	}

invalid:
	rollback_fn(context);
	return NVRAM_REHASH_INVALID;
}

#endif /* NVRAM_REHASH_POLICY_H */

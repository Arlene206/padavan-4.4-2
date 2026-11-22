#ifndef PADAVAN_WAN_NVRAM_STATUS_POLICY_H
#define PADAVAN_WAN_NVRAM_STATUS_POLICY_H

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* NVRAM APIs use 0/-1 and report the concrete failure through errno. */
static int
rc_wan_nvram_write_status(int api_result, int saved_errno)
{
	if (api_result == 0)
		return 0;
	return saved_errno > 0 ? saved_errno : EIO;
}


/* Checked NVRAM reads distinguish an absent key from a backend failure. */
static int
rc_wan_nvram_read_status(int api_result, int saved_errno)
{
	if (api_result == 0 || api_result == 1)
		return 0;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	return EIO;
}

/*
 * Match a successfully read NVRAM string exactly. The caller supplies an
 * empty value for a missing key to preserve nvram_match()'s false default.
 */
static inline int
rc_wan_nvram_match_status(int read_status, const char *value,
			  const char *expected, int *matches)
{
	if (!matches || !expected)
		return EINVAL;
	*matches = 0;
	if (read_status != 0)
		return read_status;
	if (!value)
		return EIO;
	*matches = (strcmp(value, expected) == 0);
	return 0;
}

/* Do not collapse an unreadable MAN interface into a non-WISP result. */
static inline int
rc_wan_nvram_wisp_active_status(int read_status, int validation_status,
				int is_wisp, int *active)
{
	if (!active)
		return EINVAL;
	*active = 0;
	if (read_status != 0)
		return read_status;
	if (validation_status != 0)
		return validation_status;
	*active = (is_wisp != 0);
	return 0;
}


/* Parse one complete decimal integer without atoi() overflow/prefix loss. */
static inline int
rc_wan_nvram_parse_int_value(const char *text, int *value)
{
	char *end;
	long parsed;

	if (!text || !value)
		return EINVAL;

	end = (char *)text;
	while (*end && isspace((unsigned char)*end))
		++end;
	if (!*end) {
		/* Preserve atoi()'s zero result for empty/whitespace NVRAM values. */
		*value = 0;
		return 0;
	}

	errno = 0;
	parsed = strtol(text, &end, 10);
	if (end == text)
		return EINVAL;
	if (errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX)
		return ERANGE;
	while (*end && isspace((unsigned char)*end))
		++end;
	if (*end)
		return EINVAL;

	*value = (int)parsed;
	return 0;
}

/*
 * Parse a transactionally copied integer tuple. Do not expose a partially
 * decoded configuration when one member is malformed or overflows.
 */
static inline int
rc_wan_nvram_parse_int_snapshot(const char *const *texts,
				unsigned int count, int *values)
{
	unsigned int i, j;
	int status;

	if (!texts || count == 0 || !values)
		return EINVAL;
	for (i = 0; i < count; ++i)
		values[i] = 0;

	for (i = 0; i < count; ++i) {
		status = rc_wan_nvram_parse_int_value(texts[i], &values[i]);
		if (status != 0) {
			for (j = 0; j < count; ++j)
				values[j] = 0;
			return status;
		}
	}

	return 0;
}

/*
 * nvram_get_checked() returns 0 for a missing key and -1 for a backend error.
 * Keep those states distinct while retaining the caller's legacy default.
 */
static inline int
rc_wan_nvram_read_int_status(int api_result, int saved_errno,
			     const char *text, int missing_value, int *value)
{
	int status;

	if (!value)
		return EINVAL;
	status = rc_wan_nvram_read_status(api_result, saved_errno);
	if (status != 0)
		return status;
	if (api_result == 0) {
		*value = missing_value;
		return 0;
	}
	if (!text)
		return EIO;

	return rc_wan_nvram_parse_int_value(text, value);
}

/*
 * The WAN IPv6 interface selector defaults to zero (PPP) when absent.
 * Preserve that default while keeping malformed/backend failures distinct.
 */
static inline int
rc_wan_nvram_ipv6_if_ppp_status(int api_result, int saved_errno,
				const char *text, int *is_ppp)
{
	int mode = 0;
	int status;

	if (!is_ppp)
		return EINVAL;
	*is_ppp = -1;
	status = rc_wan_nvram_read_int_status(api_result, saved_errno,
					      text, 0, &mode);
	if (status != 0)
		return status;
	*is_ppp = (mode == 0);
	return 0;
}

/*
 * Normalize a successfully read integer setting to its supported domain.
 * Read errors are returned unchanged; only readable out-of-range values use
 * the caller's historical fallback.
 */
static inline int
rc_wan_nvram_int_range_status(int read_status, int value,
			      int minimum, int maximum, int fallback,
			      int *normalized)
{
	if (!normalized || minimum > maximum ||
	    fallback < minimum || fallback > maximum)
		return EINVAL;
	if (read_status != 0)
		return read_status;

	if (value < minimum || value > maximum)
		value = fallback;
	*normalized = value;
	return 0;
}


/*
 * Transport selection affects both the PPP/NDIS stop path and interface
 * identity. Never map an NVRAM backend error to the legacy RAS default.
 */
static inline int
rc_wan_nvram_modem_is_ndis_status(int read_status, int modem_type,
				  int *is_ndis)
{
	if (!is_ndis)
		return EINVAL;
	if (read_status != 0)
		return read_status;
	*is_ndis = (modem_type == 3);
	return 0;
}


typedef int (*rc_wan_nvram_get_fn)(void *context,
					 const char *param_name,
					 char **value);

/*
 * Copy a cache-backed or caller-owned string into stable storage before an
 * NVRAM write can invalidate the source. A missing value is an empty string;
 * never truncate a value into a different network address or identifier.
 */
static inline int
rc_wan_nvram_copy_text(const char *source, char *destination,
		       size_t destination_size)
{
	size_t length;

	if (!destination || destination_size == 0)
		return EINVAL;
	if (!source)
		source = "";

	length = strlen(source);
	if (length >= destination_size) {
		destination[0] = '\0';
		return ENAMETOOLONG;
	}

	memmove(destination, source, length + 1);
	return 0;
}


/*
 * Reproduce mac_conv() formatting and fallback markers for a 12-character
 * custom WAN MAC. Historical behavior does not validate hexadecimal digits;
 * keep that compatibility while avoiding a lossy NVRAM read at the call site.
 */
static inline int
rc_wan_nvram_format_mac_override(const char *source, char *destination,
				 size_t destination_size)
{
	size_t i, output_index;
	int all_zero = 1, all_ff = 1;

	if (!destination || destination_size == 0)
		return EINVAL;
	destination[0] = '\0';
	if (!source || strlen(source) != 12)
		return 0;
	if (destination_size < 18)
		return ENAMETOOLONG;

	for (i = 0, output_index = 0; i < 12; ++i) {
		if (i != 0 && (i % 2) == 0)
			destination[output_index++] = ':';
		destination[output_index++] = source[i];
		if (source[i] != '0')
			all_zero = 0;
		if (toupper((unsigned char)source[i]) != 'F')
			all_ff = 0;
	}
	destination[output_index] = '\0';
	if (all_zero || all_ff)
		destination[0] = '\0';

	return 0;
}


/*
 * Copy one checked value before the next cache-backed NVRAM read. A missing
 * key is an empty setting; a backend failure or truncation is never hidden.
 * Callers that need a coherent tuple must hold a transaction around the reads.
 */
static inline int
rc_wan_nvram_copy_value(rc_wan_nvram_get_fn get_value, void *context,
			const char *param_name, char *destination,
			size_t destination_size)
{
	char *source = NULL;
	size_t length;
	int read_result, saved_errno, status;

	if (!get_value || !param_name || !param_name[0] ||
	    !destination || destination_size == 0)
		return EINVAL;
	destination[0] = '\0';

	errno = 0;
	read_result = get_value(context, param_name, &source);
	saved_errno = errno;
	status = rc_wan_nvram_read_status(read_result, saved_errno);
	if (status != 0)
		return status;
	if (read_result == 0)
		source = "";
	else if (!source)
		return EIO;

	length = strlen(source);
	if (length >= destination_size)
		return ENAMETOOLONG;
	memcpy(destination, source, length + 1);
	return 0;
}



typedef int (*rc_wan_nvram_get_owned_fn)(void *context,
					 const char *param_name,
					 char **value, int *present);
typedef int (*rc_wan_nvram_set_default_fn)(void *context,
					   const char *param_name,
					   const char *value);

/*
 * Seed one WAN default only when its unit-specific override is truly absent.
 * Get callbacks return an owned value (or NULL when absent) and an errno-style
 * status. This avoids lossy nvram_get() and protects cache-backed values until
 * the setter consumes them.
 */
static inline int
rc_wan_nvram_copy_missing_default(rc_wan_nvram_get_owned_fn get_owned,
				  rc_wan_nvram_set_default_fn set_default,
				  void *context, const char *target_name,
				  const char *source_name,
				  const char *fallback)
{
	char *target_value = NULL;
	char *source_value = NULL;
	const char *value;
	int present = 0;
	int status;

	if (!get_owned || !set_default || !target_name || !target_name[0] ||
	    !source_name || !source_name[0])
		return EINVAL;

	status = get_owned(context, target_name, &target_value, &present);
	if (status != 0)
		goto done;
	if (present != 0 && present != 1) {
		status = EIO;
		goto done;
	}
	if (present == 1) {
		status = target_value ? 0 : EIO;
		goto done;
	}
	if (target_value) {
		status = EIO;
		goto done;
	}

	present = 0;
	status = get_owned(context, source_name, &source_value, &present);
	if (status != 0)
		goto done;
	if (present != 0 && present != 1) {
		status = EIO;
		goto done;
	}
	if (present == 1) {
		if (!source_value) {
			status = EIO;
			goto done;
		}
		value = source_value;
	} else {
		if (source_value) {
			status = EIO;
			goto done;
		}
		value = fallback ? fallback : "";
	}

	status = set_default(context, target_name, value);

done:
	free(target_value);
	free(source_value);
	return status;
}

typedef int (*rc_wan_nvram_transaction_fn)(void *context);

struct rc_wan_nvram_snapshot_field {
	const char *param_name;
	char *destination;
	size_t destination_size;
};

/*
 * Read a bounded configuration tuple while one transaction is held.
 * Validate every field before opening the lock, copy each cache-backed value
 * before the next read, release the lock after the first read error, and
 * invalidate the whole tuple if either reading or closing fails.
 */
static inline int
rc_wan_nvram_copy_snapshot(const struct rc_wan_nvram_snapshot_field *fields,
			   unsigned int count,
			   rc_wan_nvram_transaction_fn begin_transaction,
			   rc_wan_nvram_get_fn get_value,
			   rc_wan_nvram_transaction_fn end_transaction,
			   void *context)
{
	unsigned int i;
	int api_result, saved_errno, status, end_status;

	if (!fields || count == 0 || !begin_transaction || !get_value ||
	    !end_transaction)
		return EINVAL;
	for (i = 0; i < count; ++i) {
		if (!fields[i].param_name || !fields[i].param_name[0] ||
		    !fields[i].destination || fields[i].destination_size == 0)
			return EINVAL;
	}
	for (i = 0; i < count; ++i)
		fields[i].destination[0] = '\0';

	errno = 0;
	api_result = begin_transaction(context);
	saved_errno = errno;
	status = rc_wan_nvram_write_status(api_result, saved_errno);
	if (status != 0)
		return status;

	status = 0;
	for (i = 0; i < count; ++i) {
		status = rc_wan_nvram_copy_value(
			get_value, context, fields[i].param_name,
			fields[i].destination, fields[i].destination_size);
		if (status != 0)
			break;
	}

	errno = 0;
	api_result = end_transaction(context);
	saved_errno = errno;
	end_status = rc_wan_nvram_write_status(api_result, saved_errno);
	if (status == 0)
		status = end_status;

	if (status != 0) {
		for (i = 0; i < count; ++i)
			fields[i].destination[0] = '\0';
	}
	return status;
}


typedef int (*rc_wan_nvram_param_set_fn)(void *context,
					 const char *param_name);

/*
 * Copy a required configuration tuple in order. Stop at the first failure so
 * callers can prevent activation with a partially refreshed WAN snapshot.
 */
static inline int
rc_wan_nvram_set_required_params(const char *const *param_names,
				 unsigned int count,
				 rc_wan_nvram_param_set_fn set_param,
				 void *context)
{
	unsigned int i;
	int status;

	if ((!param_names && count != 0) || !set_param)
		return EINVAL;

	for (i = 0; i < count; ++i) {
		if (!param_names[i] || !param_names[i][0])
			return EINVAL;
		status = set_param(context, param_names[i]);
		if (status != 0)
			return status;
	}

	return 0;
}


struct rc_wan_nvram_value {
	const char *param_name;
	const char *value;
};

typedef int (*rc_wan_nvram_value_set_fn)(void *context,
					 const char *param_name,
					 const char *value);

/* Clear or seed a required runtime snapshot in order, stopping at first error. */
static inline int
rc_wan_nvram_set_required_values(const struct rc_wan_nvram_value *values,
				 unsigned int count,
				 rc_wan_nvram_value_set_fn set_value,
				 void *context)
{
	unsigned int i;
	int status;

	if ((!values && count != 0) || !set_value)
		return EINVAL;

	for (i = 0; i < count; ++i) {
		if (!values[i].param_name || !values[i].param_name[0] ||
		    !values[i].value)
			return EINVAL;
		status = set_value(context, values[i].param_name, values[i].value);
		if (status != 0)
			return status;
	}

	return 0;
}


/*
 * Publish the protocol marker before the active interface identity. If the
 * protocol write fails, do not publish an interface that would imply a usable
 * WAN configuration with stale protocol metadata.
 */
static inline int
rc_wan_nvram_publish_runtime_identity(rc_wan_nvram_value_set_fn set_value,
				      void *context, const char *protocol,
				      const char *ifname)
{
	static const char protocol_name[] = "proto_t";
	static const char ifname_name[] = "ifname_t";
	struct rc_wan_nvram_value values[2];

	if (!set_value || !protocol || !protocol[0] || !ifname || !ifname[0])
		return EINVAL;

	values[0].param_name = protocol_name;
	values[0].value = protocol;
	values[1].param_name = ifname_name;
	values[1].value = ifname;

	return rc_wan_nvram_set_required_values(
		values, (unsigned int)(sizeof(values) / sizeof(values[0])),
		set_value, context);
}


/*
 * Clear independent runtime fields even after one write fails. Teardown must
 * still discard every stale route/DNS field while reporting the first error.
 */
static inline int
rc_wan_nvram_set_all_values(const struct rc_wan_nvram_value *values,
			    unsigned int count,
			    rc_wan_nvram_value_set_fn set_value,
			    void *context)
{
	unsigned int i;
	int status, first_status = 0;

	if ((!values && count != 0) || !set_value)
		return EINVAL;
	for (i = 0; i < count; ++i) {
		if (!values[i].param_name || !values[i].param_name[0] ||
		    !values[i].value)
			return EINVAL;
	}
	for (i = 0; i < count; ++i) {
		status = set_value(context, values[i].param_name, values[i].value);
		if (first_status == 0 && status != 0)
			first_status = status;
	}
	return first_status;
}


#endif /* PADAVAN_WAN_NVRAM_STATUS_POLICY_H */

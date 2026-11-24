#include <assert.h>
#include <errno.h>

#include "ipv6_service_type_policy.h"

static const char *const names[] = {
	"static", "dhcp6", "6in4", "6to4", "6rd"
};

static int
parse(int result, int saved_errno, const char *value, int *type)
{
	return ipv6_service_type_parse_status(
		result, saved_errno, value, names,
		(unsigned int)(sizeof(names) / sizeof(names[0])), 0, 1, type);
}

static void
test_missing_and_unknown_legacy_values_remain_disabled(void)
{
	int type = 99;
	assert(parse(0, 0, NULL, &type) == 0 && type == 0);
	assert(parse(1, 0, "", &type) == 0 && type == 0);
	assert(parse(1, 0, "unknown-service", &type) == 0 && type == 0);
}

static void
test_supported_services_keep_the_existing_enum_values(void)
{
	int type = -1;
	assert(parse(1, 0, "static", &type) == 0 && type == 1);
	assert(parse(1, 0, "dhcp6", &type) == 0 && type == 2);
	assert(parse(1, 0, "6in4", &type) == 0 && type == 3);
	assert(parse(1, 0, "6to4", &type) == 0 && type == 4);
	assert(parse(1, 0, "6rd", &type) == 0 && type == 5);
}

static void
test_backend_and_api_failures_are_not_reported_as_disabled(void)
{
	int type = 99;
	assert(parse(-1, EACCES, NULL, &type) == EACCES && type == 0);
	assert(parse(-1, 0, NULL, &type) == EIO && type == 0);
	assert(parse(2, 0, "static", &type) == EIO && type == 0);
	assert(parse(1, 0, NULL, &type) == EIO && type == 0);
	assert(parse(1, 0, "static", NULL) == EINVAL);
}

int
main(void)
{
	test_missing_and_unknown_legacy_values_remain_disabled();
	test_supported_services_keep_the_existing_enum_values();
	test_backend_and_api_failures_are_not_reported_as_disabled();
	return 0;
}

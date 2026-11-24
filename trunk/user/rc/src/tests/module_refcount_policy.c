#include <stdio.h>
#include <errno.h>
#include "module_refcount_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "CHECK failed at %s:%d: %s\n", \
			__FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_valid_refcounts(void)
{
	int refcount = -1;

	CHECK(module_refcount_parse("0\n", &refcount) == 0);
	CHECK(refcount == 0);
	CHECK(module_refcount_parse("42\n", &refcount) == 0);
	CHECK(refcount == 42);
	CHECK(module_refcount_parse("2147483647\n", &refcount) == 0);
	CHECK(refcount == 2147483647);
	CHECK(module_refcount_parse("17", &refcount) == 0);
	CHECK(refcount == 17);
	CHECK(module_refcount_parse("0000000001\n", &refcount) == 0);
	CHECK(refcount == 1);
	return 0;
}

static int
test_invalid_or_partial_records(void)
{
	int refcount = 99;

	CHECK(module_refcount_parse(NULL, &refcount) == EINVAL);
	CHECK(refcount == -1);
	CHECK(module_refcount_parse("", &refcount) == EINVAL);
	CHECK(refcount == -1);
	CHECK(module_refcount_parse("\n", &refcount) == EINVAL);
	CHECK(refcount == -1);
	CHECK(module_refcount_parse("-1\n", &refcount) == EINVAL);
	CHECK(refcount == -1);
	CHECK(module_refcount_parse("+1\n", &refcount) == EINVAL);
	CHECK(refcount == -1);
	CHECK(module_refcount_parse(" 1\n", &refcount) == EINVAL);
	CHECK(refcount == -1);
	CHECK(module_refcount_parse("1x\n", &refcount) == EINVAL);
	CHECK(refcount == -1);
	CHECK(module_refcount_parse("1\n2\n", &refcount) == EINVAL);
	CHECK(refcount == -1);
	CHECK(module_refcount_parse("1\n\n", &refcount) == EINVAL);
	CHECK(refcount == -1);
	CHECK(module_refcount_parse("2147483648\n", &refcount) == ERANGE);
	CHECK(refcount == -1);
	CHECK(module_refcount_parse("00000000000\n", &refcount) == ERANGE);
	CHECK(refcount == -1);
	CHECK(module_refcount_parse("12345678901", &refcount) == ERANGE);
	CHECK(refcount == -1);
	CHECK(module_refcount_parse("1", NULL) == EINVAL);
	return 0;
}

int
main(void)
{
	CHECK(test_valid_refcounts() == 0);
	CHECK(test_invalid_or_partial_records() == 0);
	puts("module refcount strict read policy: PASS");
	return 0;
}

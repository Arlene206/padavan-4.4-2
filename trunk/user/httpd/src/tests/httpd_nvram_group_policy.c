#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "httpd_nvram_group_policy.h"

#define FAKE_MAX_KEYS 512
#define FAKE_KEY_SIZE 64

struct fake_entry {
	char key[FAKE_KEY_SIZE];
	char *value;
	int used;
};

struct fake_nvram {
	struct fake_entry entries[FAKE_MAX_KEYS];
	int operation_count;
	const char *fail_key;
};

static struct fake_entry *
fake_find(struct fake_nvram *fake, const char *key, int create)
{
	int i;
	struct fake_entry *free_entry = NULL;

	for (i = 0; i < FAKE_MAX_KEYS; ++i) {
		if (fake->entries[i].used && !strcmp(fake->entries[i].key, key))
			return &fake->entries[i];
		if (!fake->entries[i].used && !free_entry)
			free_entry = &fake->entries[i];
	}
	if (!create || !free_entry)
		return NULL;
	strncpy(free_entry->key, key, sizeof(free_entry->key) - 1);
	free_entry->key[sizeof(free_entry->key) - 1] = '\0';
	free_entry->used = 1;
	return free_entry;
}

static void
fake_put(struct fake_nvram *fake, const char *key, const char *value)
{
	struct fake_entry *entry = fake_find(fake, key, 1);
	if (!entry)
		abort();
	free(entry->value);
	entry->value = strdup(value);
	if (!entry->value)
		abort();
}

static int
fake_should_fail(struct fake_nvram *fake, const char *key)
{
	++fake->operation_count;
	if (fake->fail_key && !strcmp(fake->fail_key, key)) {
		fake->fail_key = NULL;
		return 1;
	}
	return 0;
}

static const char *
fake_get(void *context, const char *key)
{
	struct fake_nvram *fake = (struct fake_nvram *)context;
	struct fake_entry *entry = fake_find(fake, key, 0);
	return (entry && entry->value) ? entry->value : NULL;
}

static int
fake_set_common(struct fake_nvram *fake, const char *key, const char *value)
{
	if (fake_should_fail(fake, key))
		return -1;
	fake_put(fake, key, value);
	return 0;
}

static int
fake_set(void *context, const char *key, const char *value)
{
	return fake_set_common((struct fake_nvram *)context, key, value);
}

static int
fake_set_temp(void *context, const char *key, const char *value)
{
	return fake_set_common((struct fake_nvram *)context, key, value);
}

static int
fake_unset(void *context, const char *key)
{
	struct fake_nvram *fake = (struct fake_nvram *)context;
	struct fake_entry *entry;

	if (fake_should_fail(fake, key))
		return -1;
	entry = fake_find(fake, key, 0);
	if (entry) {
		free(entry->value);
		entry->value = NULL;
	}
	return 0;
}

static const struct httpd_nvram_group_ops fake_ops = {
	fake_get, fake_set, fake_set_temp, fake_unset
};

static void
fake_init_group(struct fake_nvram *fake)
{
	memset(fake, 0, sizeof(*fake));
	fake_put(fake, "group_count", "4");
	fake_put(fake, "alpha0", "a0");
	fake_put(fake, "alpha1", "a1");
	fake_put(fake, "alpha2", "a2");
	fake_put(fake, "alpha3", "a3");
	fake_put(fake, "beta0", "b0");
	fake_put(fake, "beta1", "b1");
	fake_put(fake, "beta2", "b2");
	fake_put(fake, "beta3", "b3");
	fake_put(fake, "alpha_0", "old-alpha-cache");
	fake_put(fake, "beta_0", "old-beta-cache");
}

static int
expect_value(struct fake_nvram *fake, const char *key, const char *expected)
{
	const char *value = fake_get(fake, key);
	if (!expected)
		return value == NULL;
	return value && !strcmp(value, expected);
}

static int
test_single_delete(void)
{
	struct fake_nvram fake;
	const char *names[] = { "alpha", "beta" };
	int del_map[] = { 1, -1 };

	fake_init_group(&fake);
	if (httpd_nvram_group_compact(names, 2, "group_count", 4, del_map,
				      128, &fake_ops, &fake) != 0)
		return 1;
	if (!expect_value(&fake, "group_count", "3") ||
	    !expect_value(&fake, "alpha1", "a2") ||
	    !expect_value(&fake, "alpha2", "a3") ||
	    !expect_value(&fake, "beta1", "b2") ||
	    !expect_value(&fake, "beta2", "b3") ||
	    !expect_value(&fake, "alpha_0", "a1") ||
	    !expect_value(&fake, "beta_0", "b1") ||
	    !expect_value(&fake, "alpha3", NULL) ||
	    !expect_value(&fake, "beta3", NULL))
		return 1;
	return 0;
}

static int
test_multiple_delete(void)
{
	struct fake_nvram fake;
	const char *names[] = { "alpha", "beta" };
	int del_map[] = { 0, 2, -1 };

	fake_init_group(&fake);
	if (httpd_nvram_group_compact(names, 2, "group_count", 4, del_map,
				      128, &fake_ops, &fake) != 0)
		return 1;
	if (!expect_value(&fake, "group_count", "2") ||
	    !expect_value(&fake, "alpha0", "a1") ||
	    !expect_value(&fake, "alpha1", "a3") ||
	    !expect_value(&fake, "beta0", "b1") ||
	    !expect_value(&fake, "beta1", "b3") ||
	    !expect_value(&fake, "alpha_0", NULL) ||
	    !expect_value(&fake, "beta_0", NULL))
		return 1;
	return 0;
}

static int
test_invalid_map(void)
{
	struct fake_nvram fake;
	const char *names[] = { "alpha", "beta" };
	int del_map[] = { 2, 1, -1 };

	fake_init_group(&fake);
	if (httpd_nvram_group_compact(names, 2, "group_count", 4, del_map,
				      128, &fake_ops, &fake) == 0)
		return 1;
	if (!expect_value(&fake, "group_count", "4") ||
	    !expect_value(&fake, "alpha1", "a1") ||
	    !expect_value(&fake, "beta2", "b2"))
		return 1;
	return 0;
}

static int
test_later_column_failure_rolls_back(void)
{
	struct fake_nvram fake;
	const char *names[] = { "alpha", "beta" };
	int del_map[] = { 1, -1 };

	fake_init_group(&fake);
	fake.fail_key = "beta1";
	if (httpd_nvram_group_compact(names, 2, "group_count", 4, del_map,
				      128, &fake_ops, &fake) == 0)
		return 1;
	if (!expect_value(&fake, "group_count", "4") ||
	    !expect_value(&fake, "alpha0", "a0") ||
	    !expect_value(&fake, "alpha1", "a1") ||
	    !expect_value(&fake, "alpha2", "a2") ||
	    !expect_value(&fake, "alpha3", "a3") ||
	    !expect_value(&fake, "beta0", "b0") ||
	    !expect_value(&fake, "beta1", "b1") ||
	    !expect_value(&fake, "beta2", "b2") ||
	    !expect_value(&fake, "beta3", "b3") ||
	    !expect_value(&fake, "alpha_0", "old-alpha-cache") ||
	    !expect_value(&fake, "beta_0", "old-beta-cache"))
		return 1;
	return 0;
}

static int
test_deleted_value_cache_failure_rolls_back(void)
{
	struct fake_nvram fake;
	const char *names[] = { "alpha", "beta" };
	int del_map[] = { 1, -1 };

	fake_init_group(&fake);
	fake.fail_key = "beta_0";
	if (httpd_nvram_group_compact(names, 2, "group_count", 4, del_map,
				      128, &fake_ops, &fake) == 0)
		return 1;
	if (!expect_value(&fake, "group_count", "4") ||
	    !expect_value(&fake, "alpha1", "a1") ||
	    !expect_value(&fake, "beta1", "b1") ||
	    !expect_value(&fake, "alpha_0", "old-alpha-cache") ||
	    !expect_value(&fake, "beta_0", "old-beta-cache"))
		return 1;
	return 0;
}

static int
test_count_write_failure_rolls_back(void)
{
	struct fake_nvram fake;
	const char *names[] = { "alpha", "beta" };
	int del_map[] = { 3, -1 };

	fake_init_group(&fake);
	fake.fail_key = "group_count";
	if (httpd_nvram_group_compact(names, 2, "group_count", 4, del_map,
				      128, &fake_ops, &fake) == 0)
		return 1;
	if (!expect_value(&fake, "group_count", "4") ||
	    !expect_value(&fake, "alpha0", "a0") ||
	    !expect_value(&fake, "alpha3", "a3") ||
	    !expect_value(&fake, "beta0", "b0") ||
	    !expect_value(&fake, "beta3", "b3") ||
	    !expect_value(&fake, "alpha_0", "old-alpha-cache") ||
	    !expect_value(&fake, "beta_0", "old-beta-cache"))
		return 1;
	return 0;
}

static int
test_stale_cleanup_failure_rolls_back(void)
{
	struct fake_nvram fake;
	const char *names[] = { "alpha", "beta" };
	int del_map[] = { 0, 1, 2, -1 };

	fake_init_group(&fake);
	fake.fail_key = "alpha3";
	if (httpd_nvram_group_compact(names, 2, "group_count", 4, del_map,
				      128, &fake_ops, &fake) == 0)
		return 1;
	if (!expect_value(&fake, "group_count", "4") ||
	    !expect_value(&fake, "alpha0", "a0") ||
	    !expect_value(&fake, "alpha1", "a1") ||
	    !expect_value(&fake, "beta2", "b2"))
		return 1;
	return 0;
}

int
main(void)
{
	if (test_single_delete() || test_multiple_delete() || test_invalid_map() ||
	    test_later_column_failure_rolls_back() ||
	    test_deleted_value_cache_failure_rolls_back() ||
	    test_count_write_failure_rolls_back() ||
	    test_stale_cleanup_failure_rolls_back()) {
		fprintf(stderr, "NVRAM group compaction policy: FAIL\n");
		return 1;
	}
	printf("NVRAM group compaction policy: PASS\n");
	return 0;
}

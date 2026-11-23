/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <nvram_linux.h>

#include "common.h"
#include "nvram_x.h"
#include "httpd_nvram_group_policy.h"


char *nvram_get_list_x(const char *name, int index)
{
	char nv_name[64];
	int len;

	if (!name || !*name || index < 0)
		return "";

	len = snprintf(nv_name, sizeof(nv_name), "%s%d", name, index);
	if (len < 0 || len >= (int)sizeof(nv_name))
		return "";

	return nvram_safe_get(nv_name);
}

int nvram_match_list_x(const char *name, char *match, int index)
{
	const char *value;

	if (!name || !match || index < 0)
		return 0;
	value = nvram_get_list_x(name, index);
	return value && !strcmp(value, match);
}

static int
nvram_group_policy_get(void *context, const char *name, const char **value)
{
	char *nvram_value = NULL;
	int result;

	if (!context || !value)
		return -1;
	*value = NULL;
	result = nvram_transaction_get((struct nvram_transaction *)context,
				       name, &nvram_value);
	if (result == 0)
		return 0;
	if (result != 1 || !nvram_value)
		return -1;
	*value = nvram_value;
	return 1;
}

static int
nvram_group_policy_set(void *context, const char *name, const char *value)
{
	if (!context)
		return -1;
	return nvram_transaction_set((struct nvram_transaction *)context,
				     name, value, 0);
}

static int
nvram_group_policy_set_temp(void *context, const char *name, const char *value)
{
	if (!context)
		return -1;
	return nvram_transaction_set((struct nvram_transaction *)context,
				     name, value, 1);
}

static int
nvram_group_policy_unset(void *context, const char *name)
{
	if (!context)
		return -1;
	return nvram_transaction_set((struct nvram_transaction *)context,
				     name, NULL, 0);
}

int
nvram_add_group_map_x(const char * const *names, int name_count,
		      const char *count_name, const char * const *values)
{
	static const struct httpd_nvram_group_ops ops = {
		nvram_group_policy_get,
		nvram_group_policy_set,
		nvram_group_policy_set_temp,
		nvram_group_policy_unset
	};
	struct nvram_transaction transaction;
	int result, end_status;

	if (nvram_transaction_begin(&transaction) != 0) {
		fprintf(stderr, "NVRAM group add could not acquire the transaction lock\n");
		return -1;
	}
	result = httpd_nvram_group_add(names, name_count, count_name, values,
				       MAX_GROUP_COUNT, &ops, &transaction);
	end_status = nvram_transaction_end(&transaction);
	if (end_status != 0) {
		fprintf(stderr, "NVRAM group add transaction release failed (operation status=%d)\n",
			result);
		if (result == 0)
			result = -1;
	}
	if (result == -2)
		fprintf(stderr, "NVRAM group add failed and rollback was incomplete\n");
	else if (result != 0 && end_status == 0)
		fprintf(stderr, "NVRAM group add failed; group count was not increased\n");
	return result;
}

int
nvram_del_group_map_x(const char * const *names, int name_count,
		      const char *count_name, int group_count, int *delMap)
{
	static const struct httpd_nvram_group_ops ops = {
		nvram_group_policy_get,
		nvram_group_policy_set,
		nvram_group_policy_set_temp,
		nvram_group_policy_unset
	};
	struct nvram_transaction transaction;
	int result, end_status;

	if (nvram_transaction_begin(&transaction) != 0) {
		fprintf(stderr, "NVRAM group compaction could not acquire the transaction lock\n");
		return -1;
	}
	result = httpd_nvram_group_compact(names, name_count, count_name,
					   group_count, delMap,
					   MAX_GROUP_COUNT, &ops, &transaction);
	end_status = nvram_transaction_end(&transaction);
	if (end_status != 0) {
		fprintf(stderr, "NVRAM group compaction transaction release failed (operation status=%d)\n",
			result);
		if (result == 0)
			result = -1;
	}
	if (result == -2)
		fprintf(stderr, "NVRAM group compaction failed and rollback was incomplete; group count was not reduced\n");
	else if (result != 0 && end_status == 0)
		fprintf(stderr, "NVRAM group compaction failed; group count was not reduced\n");
	return result;
}

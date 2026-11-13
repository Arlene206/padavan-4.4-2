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

static const char *
nvram_group_policy_get(void *context, const char *name)
{
	(void)context;
	return nvram_get(name);
}

static int
nvram_group_policy_set(void *context, const char *name, const char *value)
{
	(void)context;
	return nvram_set(name, value);
}

static int
nvram_group_policy_set_temp(void *context, const char *name, const char *value)
{
	(void)context;
	return nvram_set_temp(name, value);
}

static int
nvram_group_policy_unset(void *context, const char *name)
{
	(void)context;
	return nvram_unset(name);
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
	int result;

	result = httpd_nvram_group_compact(names, name_count, count_name,
					   group_count, delMap,
					   MAX_GROUP_COUNT, &ops, NULL);
	if (result == -2)
		fprintf(stderr, "NVRAM group compaction failed and rollback was incomplete; group count was not reduced\n");
	else if (result != 0)
		fprintf(stderr, "NVRAM group compaction failed; group count was not reduced\n");
	return result;
}

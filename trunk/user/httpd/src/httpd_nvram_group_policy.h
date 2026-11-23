#ifndef HTTPD_NVRAM_GROUP_POLICY_H
#define HTTPD_NVRAM_GROUP_POLICY_H

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HTTPD_NVRAM_GROUP_KEY_SIZE 64
#define HTTPD_NVRAM_GROUP_EXTRA_ROWS 8

struct httpd_nvram_group_ops {
	/* 1=present, 0=missing, -1=backend/read error. */
	int (*get)(void *context, const char *name, const char **value);
	int (*set)(void *context, const char *name, const char *value);
	int (*set_temp)(void *context, const char *name, const char *value);
	int (*unset)(void *context, const char *name);
};

struct httpd_nvram_group_cell {
	char *value;
	int present;
};

static int
httpd_nvram_group_make_key(char *key, size_t key_size,
			   const char *prefix, int index)
{
	int length;

	length = snprintf(key, key_size, "%s%d", prefix, index);
	return (length >= 0 && (size_t)length < key_size) ? 0 : -1;
}

/*
 * The caller's group count is sampled before nvram_del_group_map_x() obtains
 * the NVRAM transaction lock. A concurrent add/remove can make that sample
 * stale; compare it with the count read under the lock before moving any row.
 */
static int
httpd_nvram_group_count_parse(const char *text, int maximum, int *value)
{
	char *end;
	long parsed;

	if (!text || !value || maximum < 0)
		return 0;

	errno = 0;
	parsed = strtol(text, &end, 10);
	if (end == text || errno == ERANGE || parsed < 0 || parsed > maximum)
		return 0;
	while (*end && isspace((unsigned char)*end))
		++end;
	if (*end != '\0')
		return 0;

	*value = (int)parsed;
	return 1;
}

static int
httpd_nvram_group_count_matches(const char *text, int expected)
{
	int parsed;

	return expected >= 0 &&
	       httpd_nvram_group_count_parse(text, INT_MAX, &parsed) &&
	       parsed == expected;
}

static void
httpd_nvram_group_free_cells(struct httpd_nvram_group_cell *cells,
			     size_t cell_count)
{
	size_t i;

	if (!cells)
		return;
	for (i = 0; i < cell_count; ++i)
		free(cells[i].value);
	free(cells);
}

static int
httpd_nvram_group_restore(const char * const *names, int name_count,
			  int row_slots,
			  struct httpd_nvram_group_cell *cells,
			  struct httpd_nvram_group_cell *last_cells,
			  const char *count_name, const char *count_value,
			  const struct httpd_nvram_group_ops *ops,
			  void *context)
{
	char key[HTTPD_NVRAM_GROUP_KEY_SIZE];
	int column, row, failed = 0, length;

	for (column = 0; column < name_count; ++column) {
		for (row = 0; row < row_slots; ++row) {
			struct httpd_nvram_group_cell *cell;

			if (httpd_nvram_group_make_key(key, sizeof(key),
						       names[column], row) != 0) {
				failed = 1;
				continue;
			}
			cell = &cells[(size_t)column * (size_t)row_slots +
				      (size_t)row];
			if (cell->present) {
				if (ops->set(context, key, cell->value) != 0)
					failed = 1;
			} else if (ops->unset(context, key) != 0) {
				failed = 1;
			}
		}

		length = snprintf(key, sizeof(key), "%s_0", names[column]);
		if (length < 0 || (size_t)length >= sizeof(key)) {
			failed = 1;
			continue;
		}
		if (last_cells[column].present) {
			if (ops->set_temp(context, key,
					  last_cells[column].value) != 0)
				failed = 1;
		} else if (ops->unset(context, key) != 0) {
			failed = 1;
		}
	}

	if (ops->set(context, count_name, count_value) != 0)
		failed = 1;
	return failed ? -1 : 0;
}

/*
 * Restore one append destination, each deleted-value cache and the count.
 * The caller still holds the NVRAM transaction lock.
 */
static int
httpd_nvram_group_add_restore(const char * const *names, int name_count,
			      int group_index,
			      struct httpd_nvram_group_cell *cells,
			      struct httpd_nvram_group_cell *last_cells,
			      const char *count_name,
			      const char *count_snapshot, int count_present,
			      const struct httpd_nvram_group_ops *ops,
			      void *context)
{
	char key[HTTPD_NVRAM_GROUP_KEY_SIZE];
	int column, length, failed = 0;

	for (column = 0; column < name_count; ++column) {
		if (httpd_nvram_group_make_key(key, sizeof(key), names[column],
					       group_index) != 0) {
			failed = 1;
		} else if (cells[column].present) {
			if (ops->set(context, key, cells[column].value) != 0)
				failed = 1;
		} else if (ops->unset(context, key) != 0) {
			failed = 1;
		}

		length = snprintf(key, sizeof(key), "%s_0", names[column]);
		if (length < 0 || (size_t)length >= sizeof(key)) {
			failed = 1;
			continue;
		}
		if (last_cells[column].present) {
			if (ops->set_temp(context, key,
					  last_cells[column].value) != 0)
				failed = 1;
		} else if (ops->unset(context, key) != 0) {
			failed = 1;
		}
	}

	if (count_present) {
		if (ops->set(context, count_name, count_snapshot) != 0)
			failed = 1;
	} else if (ops->unset(context, count_name) != 0) {
		failed = 1;
	}
	return failed ? -1 : 0;
}

/*
 * Append one complete row while the caller holds the NVRAM transaction.
 * Missing legacy counts start at zero. Snapshot every destination before
 * mutation, clear the last-deleted cache, and publish the count last.
 * Return 0 on success, -1 on failure with successful rollback, or -2 if
 * rollback was incomplete.
 */
static int
httpd_nvram_group_add(const char * const *names, int name_count,
		      const char *count_name, const char * const *values,
		      int max_group_count,
		      const struct httpd_nvram_group_ops *ops, void *context)
{
	struct httpd_nvram_group_cell *cells = NULL, *last_cells = NULL;
	char key[HTTPD_NVRAM_GROUP_KEY_SIZE];
	char count_value[16];
	char *count_snapshot = NULL;
	const char *stored_count = NULL;
	int count_status, count_present = 0, group_count = 0, new_count;
	int column, other, length, rollback_result;
	int result = -1;

	if (!names || name_count < 1 || !count_name || !*count_name ||
	    strlen(count_name) >= sizeof(key) || !values ||
	    !ops || !ops->get || !ops->set || !ops->set_temp || !ops->unset ||
	    !context || max_group_count < 1)
		return -1;

	count_status = ops->get(context, count_name, &stored_count);
	if (count_status < 0 || count_status > 1 ||
	    (count_status == 1 && !stored_count))
		return -1;
	if (count_status == 1) {
		count_present = 1;
		count_snapshot = strdup(stored_count);
		if (!count_snapshot)
			return -1;
		if (!httpd_nvram_group_count_parse(stored_count, max_group_count,
						   &group_count))
			goto done;
	}
	if (group_count >= max_group_count)
		goto done;

	for (column = 0; column < name_count; ++column) {
		if (!names[column] || !*names[column] || !values[column] ||
		    !strcmp(names[column], count_name) ||
		    strlen(names[column]) + 3 >= sizeof(key) ||
		    httpd_nvram_group_make_key(key, sizeof(key), names[column],
					       group_count) != 0 ||
		    !strcmp(key, count_name)) 
			goto done;
		for (other = 0; other < column; ++other) {
			if (!strcmp(names[column], names[other]))
				goto done;
		}
		length = snprintf(key, sizeof(key), "%s_0", names[column]);
		if (length < 0 || (size_t)length >= sizeof(key) ||
		    !strcmp(key, count_name))
			goto done;
	}

	cells = (struct httpd_nvram_group_cell *)calloc(
		(size_t)name_count, sizeof(*cells));
	last_cells = (struct httpd_nvram_group_cell *)calloc(
		(size_t)name_count, sizeof(*last_cells));
	if (!cells || !last_cells)
		goto done;

	/* Snapshot all append destinations and cache cells before any write. */
	for (column = 0; column < name_count; ++column) {
		const char *value = NULL;
		int get_status;

		if (httpd_nvram_group_make_key(key, sizeof(key), names[column],
					       group_count) != 0)
			goto done;
		get_status = ops->get(context, key, &value);
		if (get_status < 0 || get_status > 1 ||
		    (get_status == 1 && !value))
			goto done;
		if (get_status == 1) {
			cells[column].value = strdup(value);
			if (!cells[column].value)
				goto done;
			cells[column].present = 1;
		}

		length = snprintf(key, sizeof(key), "%s_0", names[column]);
		if (length < 0 || (size_t)length >= sizeof(key))
			goto done;
		value = NULL;
		get_status = ops->get(context, key, &value);
		if (get_status < 0 || get_status > 1 ||
		    (get_status == 1 && !value))
			goto done;
		if (get_status == 1) {
			last_cells[column].value = strdup(value);
			if (!last_cells[column].value)
				goto done;
			last_cells[column].present = 1;
		}
	}

	new_count = group_count + 1;
	length = snprintf(count_value, sizeof(count_value), "%d", new_count);
	if (length < 0 || (size_t)length >= sizeof(count_value))
		goto done;

	/* A failed write may have taken effect: rollback every saved key. */
	for (column = 0; column < name_count; ++column) {
		length = snprintf(key, sizeof(key), "%s_0", names[column]);
		if (length < 0 || (size_t)length >= sizeof(key))
			goto rollback;
		if (ops->unset(context, key) != 0)
			goto rollback;
		if (httpd_nvram_group_make_key(key, sizeof(key), names[column],
					       group_count) != 0)
			goto rollback;
		if (ops->set(context, key, values[column]) != 0)
			goto rollback;
	}

	/* Publish the count only after every group column is written. */
	if (ops->set(context, count_name, count_value) != 0)
		goto rollback;

	result = 0;
	goto done;

rollback:
	rollback_result = httpd_nvram_group_add_restore(
		names, name_count, group_count, cells, last_cells, count_name,
		count_snapshot, count_present, ops, context);
	result = rollback_result == 0 ? -1 : -2;

done:
	httpd_nvram_group_free_cells(cells, (size_t)name_count);
	httpd_nvram_group_free_cells(last_cells, (size_t)name_count);
	free(count_snapshot);
	return result;
}

/*
 * Compact all columns as one bounded operation. The group count is written
 * only after every column and stale-row cleanup succeeds. On failure, restore
 * the pre-operation snapshot on a best-effort basis. Return 0 on success,
 * -1 on failure with successful rollback, or -2 when rollback also failed.
 */
static int
httpd_nvram_group_compact(const char * const *names, int name_count,
			 const char *count_name, int group_count,
			 const int *del_map, int max_group_count,
			 const struct httpd_nvram_group_ops *ops,
			 void *context)
{
	struct httpd_nvram_group_cell *cells = NULL;
	struct httpd_nvram_group_cell *last_cells = NULL;
	char key[HTTPD_NVRAM_GROUP_KEY_SIZE];
	char count_value[16];
	char *count_snapshot = NULL;
	size_t cell_count, column_offset;
	int map_count = 0, previous = -1;
	int row_slots, new_count;
	int column, row, deleted_index, length;
	int rollback_result;

	if (!names || name_count < 1 || !count_name || !*count_name ||
	    strlen(count_name) >= sizeof(key) || !del_map ||
	    !ops || !ops->get || !ops->set || !ops->set_temp || !ops->unset ||
	    max_group_count < 1 || group_count < 1 ||
	    group_count > max_group_count)
		return -1;

	while (map_count <= max_group_count && del_map[map_count] != -1) {
		if (map_count >= group_count || del_map[map_count] < 0 ||
		    del_map[map_count] >= group_count ||
		    del_map[map_count] <= previous)
			return -1;
		previous = del_map[map_count];
		++map_count;
	}
	if (map_count > max_group_count || del_map[map_count] != -1)
		return -1;
	if (map_count == 0)
		return 0;

	{
		const char *stored_count = NULL;
		int get_status;

		get_status = ops->get(context, count_name, &stored_count);
		if (get_status != 1 || !stored_count ||
		    !httpd_nvram_group_count_matches(stored_count, group_count))
			return -1;
		count_snapshot = strdup(stored_count);
		if (!count_snapshot)
			return -1;
	}

	for (column = 0; column < name_count; ++column) {
		if (!names[column] || !*names[column] ||
		    strlen(names[column]) + 3 >= sizeof(key)) {
			free(count_snapshot);
			return -1;
		}
		for (row = 0; row < column; ++row) {
			if (!strcmp(names[column], names[row])) {
				free(count_snapshot);
				return -1;
			}
		}
	}

	row_slots = group_count + HTTPD_NVRAM_GROUP_EXTRA_ROWS;
	if ((size_t)name_count > ((size_t)-1) / (size_t)row_slots) {
		free(count_snapshot);
		return -1;
	}
	cell_count = (size_t)name_count * (size_t)row_slots;
	cells = (struct httpd_nvram_group_cell *)calloc(cell_count,
							 sizeof(*cells));
	last_cells = (struct httpd_nvram_group_cell *)calloc(
		(size_t)name_count, sizeof(*last_cells));
	if (!cells || !last_cells)
		goto fail_before_write;

	/* Snapshot every live row and the cleanup tail before any mutation. */
	for (column = 0; column < name_count; ++column) {
		for (row = 0; row < row_slots; ++row) {
			struct httpd_nvram_group_cell *cell;
			const char *value = NULL;
			int get_status;

			if (httpd_nvram_group_make_key(key, sizeof(key),
						       names[column], row) != 0)
				goto fail_before_write;
			get_status = ops->get(context, key, &value);
			if (get_status < 0 || get_status > 1 ||
			    (get_status == 1 && !value))
				goto fail_before_write;
			cell = &cells[(size_t)column * (size_t)row_slots +
				      (size_t)row];
			if (get_status == 1) {
				cell->value = strdup(value);
				if (!cell->value)
					goto fail_before_write;
				cell->present = 1;
			}
		}

		length = snprintf(key, sizeof(key), "%s_0", names[column]);
		if (length < 0 || (size_t)length >= sizeof(key))
			goto fail_before_write;
		{
			const char *value = NULL;
			int get_status;

			get_status = ops->get(context, key, &value);
			if (get_status < 0 || get_status > 1 ||
			    (get_status == 1 && !value))
				goto fail_before_write;
			if (get_status == 1) {
				last_cells[column].value = strdup(value);
				if (!last_cells[column].value)
					goto fail_before_write;
				last_cells[column].present = 1;
			}
		}
	}

	new_count = group_count - map_count;
	for (column = 0; column < name_count; ++column) {
		int next_index = 0;
		int delete_index = 0;

		column_offset = (size_t)column * (size_t)row_slots;
		for (row = 0; row < group_count; ++row) {
			struct httpd_nvram_group_cell *cell;
			const char *value;

			if (delete_index < map_count && del_map[delete_index] == row) {
				++delete_index;
				continue;
			}
			if (httpd_nvram_group_make_key(key, sizeof(key),
						       names[column], next_index) != 0)
				goto fail_after_write;
			cell = &cells[column_offset + (size_t)row];
			value = cell->present ? cell->value : "";
			if (ops->set(context, key, value) != 0)
				goto fail_after_write;
			++next_index;
		}

		length = snprintf(key, sizeof(key), "%s_0", names[column]);
		if (length < 0 || (size_t)length >= sizeof(key))
			goto fail_after_write;
		if (map_count == 1) {
			deleted_index = del_map[0];
			{
				struct httpd_nvram_group_cell *cell;
				const char *value;

				cell = &cells[column_offset + (size_t)deleted_index];
				value = cell->present ? cell->value : "";
				if (ops->set_temp(context, key, value) != 0)
					goto fail_after_write;
			}
		} else if (ops->unset(context, key) != 0) {
			goto fail_after_write;
		}
	}

	/* Do not remove old rows until all compacted columns have been written. */
	for (column = 0; column < name_count; ++column) {
		for (row = row_slots - 1; row >= new_count; --row) {
			if (httpd_nvram_group_make_key(key, sizeof(key),
						       names[column], row) != 0)
				goto fail_after_write;
			if (ops->unset(context, key) != 0)
				goto fail_after_write;
		}
	}

	length = snprintf(count_value, sizeof(count_value), "%d", new_count);
	if (length < 0 || (size_t)length >= sizeof(count_value))
		goto fail_after_write;
	if (ops->set(context, count_name, count_value) != 0)
		goto fail_after_write;

	httpd_nvram_group_free_cells(cells, cell_count);
	httpd_nvram_group_free_cells(last_cells, (size_t)name_count);
	free(count_snapshot);
	return 0;

fail_after_write:
	rollback_result = httpd_nvram_group_restore(names, name_count, row_slots,
						     cells, last_cells,
						     count_name, count_snapshot,
						     ops, context);
	httpd_nvram_group_free_cells(cells, cell_count);
	httpd_nvram_group_free_cells(last_cells, (size_t)name_count);
	free(count_snapshot);
	return rollback_result == 0 ? -1 : -2;

fail_before_write:
	httpd_nvram_group_free_cells(cells, cell_count);
	httpd_nvram_group_free_cells(last_cells, (size_t)name_count);
	free(count_snapshot);
	return -1;
}

#endif /* HTTPD_NVRAM_GROUP_POLICY_H */

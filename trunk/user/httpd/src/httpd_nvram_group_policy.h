#ifndef HTTPD_NVRAM_GROUP_POLICY_H
#define HTTPD_NVRAM_GROUP_POLICY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HTTPD_NVRAM_GROUP_KEY_SIZE 64
#define HTTPD_NVRAM_GROUP_EXTRA_ROWS 8

struct httpd_nvram_group_ops {
	const char *(*get)(void *context, const char *name);
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
		const char *stored_count = ops->get(context, count_name);
		if (!stored_count)
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
			const char *value;

			if (httpd_nvram_group_make_key(key, sizeof(key),
						       names[column], row) != 0)
				goto fail_before_write;
			value = ops->get(context, key);
			cell = &cells[(size_t)column * (size_t)row_slots +
				      (size_t)row];
			if (value) {
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
			const char *value = ops->get(context, key);
			if (value) {
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

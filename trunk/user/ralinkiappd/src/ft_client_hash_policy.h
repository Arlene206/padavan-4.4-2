#ifndef FT_CLIENT_HASH_POLICY_H
#define FT_CLIENT_HASH_POLICY_H

#include <stddef.h>

/*
 * The FT client table is a fixed-size slot array. A released slot may still
 * carry a next pointer from its former hash chain; clear that link before
 * making the slot available to another bucket.
 */
static inline FT_CLIENT_INFO *
ft_client_hash_find_free_slot(FT_CLIENT_INFO *entries, size_t capacity)
{
	size_t i;

	if (entries == NULL || capacity == 0)
		return NULL;

	for (i = 0; i < capacity; i++) {
		if (!entries[i].used) {
			entries[i].next = NULL;
			return &entries[i];
		}
	}

	return NULL;
}

/* Append only to a bounded, acyclic chain and never insert an entry twice. */
static inline int
ft_client_hash_append(FT_CLIENT_INFO **head, FT_CLIENT_INFO *entry,
		      size_t capacity)
{
	FT_CLIENT_INFO **cursor;
	size_t visited = 0;

	if (head == NULL || entry == NULL || capacity == 0)
		return 0;

	cursor = head;
	while (*cursor != NULL) {
		if (*cursor == entry || visited >= capacity)
			return 0;
		cursor = &(*cursor)->next;
		visited++;
	}

	if (visited >= capacity)
		return 0;

	entry->next = NULL;
	*cursor = entry;
	return 1;
}

/* Remove an entry and sever its old link before its fixed slot is reused. */
static inline int
ft_client_hash_unlink(FT_CLIENT_INFO **head, FT_CLIENT_INFO *entry,
		      size_t capacity)
{
	FT_CLIENT_INFO **cursor;
	size_t visited = 0;

	if (head == NULL || entry == NULL || capacity == 0)
		return 0;

	cursor = head;
	while (*cursor != NULL && visited < capacity) {
		if (*cursor == entry) {
			*cursor = entry->next;
			entry->next = NULL;
			return 1;
		}

		cursor = &(*cursor)->next;
		visited++;
	}

	return 0;
}

#endif /* FT_CLIENT_HASH_POLICY_H */

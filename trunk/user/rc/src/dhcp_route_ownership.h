/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PADAVAN_DHCP_ROUTE_OWNERSHIP_H
#define PADAVAN_DHCP_ROUTE_OWNERSHIP_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DHCP_ROUTE_OWNERSHIP_MAX_RECORDS 512U
#define DHCP_ROUTE_OWNERSHIP_MAGIC "PADAVAN-DHCP-ROUTES 2"

struct dhcp_route_ownership_entry {
	char ifname[IFNAMSIZ];
	char owner[16];
	int metric;
	char destination[INET_ADDRSTRLEN];
	char gateway[INET_ADDRSTRLEN];
	char netmask[INET_ADDRSTRLEN];
};

typedef int (*dhcp_route_ownership_operation)(
	void *context, const struct dhcp_route_ownership_entry *entry);

static inline int
dhcp_route_ownership_ifname_is_valid(const char *ifname)
{
	const unsigned char *p = (const unsigned char *)ifname;
	size_t length = 0;

	if (!p || !*p)
		return 0;
	for (; *p; ++p, ++length) {
		if (length >= IFNAMSIZ - 1)
			return 0;
		if (!((*p >= 'a' && *p <= 'z') ||
		      (*p >= 'A' && *p <= 'Z') ||
		      (*p >= '0' && *p <= '9') ||
		      *p == '_' || *p == '.' || *p == ':' || *p == '-'))
			return 0;
	}
	return 1;
}

static inline int
dhcp_route_ownership_owner_is_valid(const char *owner)
{
	const unsigned char *p = (const unsigned char *)owner;
	size_t length = 0;

	if (!p || !*p)
		return 0;
	for (; *p; ++p, ++length) {
		if (length >= 15U)
			return 0;
		if (!((*p >= 'a' && *p <= 'z') ||
		      (*p >= 'A' && *p <= 'Z') ||
		      (*p >= '0' && *p <= '9') || *p == '_'))
			return 0;
	}
	return owner[length - 1U] == '_';
}

static inline int
dhcp_route_ownership_ipv4_is_canonical(const char *text)
{
	struct in_addr address;
	char canonical[INET_ADDRSTRLEN];

	if (!text || inet_pton(AF_INET, text, &address) != 1)
		return 0;
	if (!inet_ntop(AF_INET, &address, canonical, sizeof(canonical)))
		return 0;
	return strcmp(text, canonical) == 0;
}

static inline int
dhcp_route_ownership_entry_is_valid(
	const struct dhcp_route_ownership_entry *entry)
{
	return entry != NULL &&
	       dhcp_route_ownership_ifname_is_valid(entry->ifname) &&
	       dhcp_route_ownership_owner_is_valid(entry->owner) &&
	       entry->metric >= 0 && entry->metric < INT_MAX &&
	       dhcp_route_ownership_ipv4_is_canonical(entry->destination) &&
	       dhcp_route_ownership_ipv4_is_canonical(entry->gateway) &&
	       dhcp_route_ownership_ipv4_is_canonical(entry->netmask);
}

static inline int
dhcp_route_ownership_entry_matches(
	const struct dhcp_route_ownership_entry *left,
	const struct dhcp_route_ownership_entry *right)
{
	return left && right &&
	       left->metric == right->metric &&
	       strcmp(left->ifname, right->ifname) == 0 &&
	       strcmp(left->owner, right->owner) == 0 &&
	       strcmp(left->destination, right->destination) == 0 &&
	       strcmp(left->gateway, right->gateway) == 0 &&
	       strcmp(left->netmask, right->netmask) == 0;
}

static inline int
dhcp_route_ownership_lock(const char *lock_path)
{
	int fd, saved_errno;
	struct stat st;

	fd = open(lock_path, O_RDWR | O_CREAT | O_NOFOLLOW | O_CLOEXEC, 0600);
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
	    st.st_nlink != 1 || (st.st_mode & 077) != 0) {
		errno = !S_ISREG(st.st_mode) ? EINVAL :
			(st.st_uid != geteuid() ? EPERM :
			 (st.st_nlink != 1 ? EMLINK : EACCES));
		goto failed;
	}
	while (flock(fd, LOCK_EX) != 0) {
		if (errno == EINTR)
			continue;
		goto failed;
	}
	return fd;

failed:
	saved_errno = errno ? errno : EIO;
	(void)close(fd);
	errno = saved_errno;
	return -1;
}

static inline int
dhcp_route_ownership_unlock(int fd)
{
	int status = 0, saved_errno = 0;

	if (fd < 0) {
		errno = EINVAL;
		return EINVAL;
	}
	if (flock(fd, LOCK_UN) != 0) {
		status = errno ? errno : EIO;
		saved_errno = status;
	}
	if (close(fd) != 0 && status == 0) {
		status = errno ? errno : EIO;
		saved_errno = status;
	}
	if (status != 0)
		errno = saved_errno;
	return status;
}

static inline int
dhcp_route_ownership_load(
	const char *path,
	struct dhcp_route_ownership_entry *entries,
	size_t capacity,
	size_t *count)
{
	int fd, status = 0, saved_errno = 0, parsed;
	char extra;
	struct stat st;
	FILE *stream;
	char line[128], canonical[128];
	struct dhcp_route_ownership_entry entry;
	size_t used = 0, i;

	if (!path || !path[0] || !entries || !count || capacity == 0)
		return EINVAL;
	*count = 0;
	fd = open(path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC);
	if (fd < 0)
		return errno == ENOENT ? 0 : (errno ? errno : EIO);
	if (fstat(fd, &st) != 0) {
		status = errno ? errno : EIO;
		goto close_fd;
	}
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
	    st.st_nlink != 1 || (st.st_mode & 077) != 0) {
		status = !S_ISREG(st.st_mode) ? EINVAL :
			(st.st_uid != geteuid() ? EPERM :
			 (st.st_nlink != 1 ? EMLINK : EACCES));
		goto close_fd;
	}
	if (st.st_size < 0 ||
	    (unsigned long long)st.st_size >
	    (unsigned long long)capacity * sizeof(line)) {
		status = EOVERFLOW;
		goto close_fd;
	}
	stream = fdopen(fd, "r");
	if (!stream) {
		status = errno ? errno : EIO;
		goto close_fd;
	}
	fd = -1;
	if (!fgets(line, sizeof(line), stream) ||
	    strcmp(line, DHCP_ROUTE_OWNERSHIP_MAGIC "\n") != 0) {
		status = EINVAL;
		goto close_stream;
	}

	while (fgets(line, sizeof(line), stream) != NULL) {
		char *newline = strchr(line, '\n');

		if (!newline || newline[1] != '\0') {
			status = EINVAL;
			goto close_stream;
		}
		*newline = '\0';
		memset(&entry, 0, sizeof(entry));
		extra = 0;
		parsed = sscanf(line, "%15s %15s %d %15s %15s %15s %c",
				entry.ifname, entry.owner, &entry.metric,
				entry.destination, entry.gateway, entry.netmask,
				&extra);
		if (parsed != 6 || !dhcp_route_ownership_entry_is_valid(&entry)) {
			status = EINVAL;
			goto close_stream;
		}
		if (snprintf(canonical, sizeof(canonical), "%s %s %d %s %s %s",
			     entry.ifname, entry.owner, entry.metric,
			     entry.destination, entry.gateway, entry.netmask) < 0 ||
		    strcmp(canonical, line) != 0) {
			status = EINVAL;
			goto close_stream;
		}
		if (used >= capacity) {
			status = ENOSPC;
			goto close_stream;
		}
		for (i = 0; i < used; ++i) {
			if (dhcp_route_ownership_entry_matches(&entries[i], &entry)) {
				status = EEXIST;
				goto close_stream;
			}
		}
		entries[used++] = entry;
	}
	if (ferror(stream))
		status = errno ? errno : EIO;

close_stream:
	saved_errno = errno;
	if (fclose(stream) != 0 && status == 0)
		status = errno ? errno : EIO;
	if (status == 0)
		*count = used;
	else
		errno = saved_errno ? saved_errno : status;
	return status;

close_fd:
	saved_errno = errno;
	if (close(fd) != 0 && status == 0)
		status = errno ? errno : EIO;
	if (status != 0)
		errno = saved_errno ? saved_errno : status;
	return status;
}

static inline int
dhcp_route_ownership_save(
	const char *path,
	const struct dhcp_route_ownership_entry *entries,
	size_t count)
{
	static const char suffix[] = ".tmp.XXXXXX";
	char *staged_path;
	struct stat st;
	FILE *stream;
	int fd = -1, status = 0, saved_errno = 0;
	size_t path_length, i;

	if (!path || !path[0] || (count > 0 && !entries) ||
	    count > DHCP_ROUTE_OWNERSHIP_MAX_RECORDS)
		return EINVAL;
	if (lstat(path, &st) == 0) {
		if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
		    st.st_nlink != 1 || (st.st_mode & 077) != 0)
			return !S_ISREG(st.st_mode) ? EINVAL :
				(st.st_uid != geteuid() ? EPERM :
				 (st.st_nlink != 1 ? EMLINK : EACCES));
	} else if (errno != ENOENT) {
		return errno ? errno : EIO;
	}

	path_length = strlen(path);
	if (path_length > (size_t)-1 - sizeof(suffix))
		return EOVERFLOW;
	staged_path = (char *)malloc(path_length + sizeof(suffix));
	if (!staged_path)
		return ENOMEM;
	if (snprintf(staged_path, path_length + sizeof(suffix),
		     "%s%s", path, suffix) < 0) {
		status = EIO;
		goto out;
	}
	fd = mkstemp(staged_path);
	if (fd < 0) {
		status = errno ? errno : EIO;
		goto out;
	}
	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0) {
		status = errno ? errno : EIO;
		goto out;
	}
	stream = fdopen(fd, "w");
	if (!stream) {
		status = errno ? errno : EIO;
		goto out;
	}
	fd = -1;
	if (fprintf(stream, "%s\n", DHCP_ROUTE_OWNERSHIP_MAGIC) < 0)
		status = errno ? errno : EIO;
	for (i = 0; status == 0 && i < count; ++i) {
		if (!dhcp_route_ownership_entry_is_valid(&entries[i])) {
			status = EINVAL;
			break;
		}
		if (fprintf(stream, "%s %s %d %s %s %s\n",
			    entries[i].ifname, entries[i].owner, entries[i].metric,
			    entries[i].destination, entries[i].gateway,
			    entries[i].netmask) < 0)
			status = errno ? errno : EIO;
	}
	if (status == 0 && fflush(stream) != 0)
		status = errno ? errno : EIO;
	if (status == 0 && fsync(fileno(stream)) != 0)
		status = errno ? errno : EIO;
	if (fclose(stream) != 0 && status == 0)
		status = errno ? errno : EIO;
	if (status != 0)
		goto out;
	if (rename(staged_path, path) != 0) {
		status = errno ? errno : EIO;
		goto out;
	}

out:
	saved_errno = errno;
	if (fd >= 0)
		(void)close(fd);
	if (status != 0)
		(void)unlink(staged_path);
	free(staged_path);
	if (status != 0)
		errno = saved_errno ? saved_errno : status;
	return status;
}

/*
 * Record only routes newly created by this DHCP owner. EEXIST remains an
 * idempotent install result, but an untracked pre-existing route is never
 * claimed and therefore cannot be deleted by a later DHCP teardown.
 */
static inline int
dhcp_route_ownership_add(
	const char *path,
	const struct dhcp_route_ownership_entry *entry,
	dhcp_route_ownership_operation add_operation,
	dhcp_route_ownership_operation delete_operation,
	void *context)
{
	struct dhcp_route_ownership_entry entries[DHCP_ROUTE_OWNERSHIP_MAX_RECORDS];
	char *lock_path;
	size_t count = 0, i, path_length;
	int lock_fd = -1, status, saved_errno;

	if (!path || !path[0] || !dhcp_route_ownership_entry_is_valid(entry) ||
	    !add_operation || !delete_operation)
		return EINVAL;
	path_length = strlen(path);
	if (path_length > (size_t)-1 - sizeof(".lock"))
		return EOVERFLOW;
	lock_path = (char *)malloc(path_length + sizeof(".lock"));
	if (!lock_path)
		return ENOMEM;
	(void)snprintf(lock_path, path_length + sizeof(".lock"), "%s.lock", path);
	lock_fd = dhcp_route_ownership_lock(lock_path);
	free(lock_path);
	if (lock_fd < 0)
		return errno ? errno : EIO;

	status = dhcp_route_ownership_load(path, entries,
					   DHCP_ROUTE_OWNERSHIP_MAX_RECORDS,
					   &count);
	if (status != 0)
		goto out;
	for (i = 0; i < count; ++i) {
		if (dhcp_route_ownership_entry_matches(&entries[i], entry))
			break;
	}
	status = add_operation(context, entry);
	if (status == EEXIST) {
		/* Existing manifest record means the route was ours in an earlier
		 * callback; an untracked EEXIST belongs to somebody else. */
		status = 0;
		goto out;
	}
	if (status != 0 || i < count)
		goto out;
	if (count >= DHCP_ROUTE_OWNERSHIP_MAX_RECORDS) {
		status = ENOSPC;
		(void)delete_operation(context, entry);
		goto out;
	}
	entries[count++] = *entry;
	status = dhcp_route_ownership_save(path, entries, count);
	if (status != 0) {
		saved_errno = status;
		(void)delete_operation(context, entry);
		status = saved_errno;
	}

out:
	if (lock_fd >= 0) {
		saved_errno = dhcp_route_ownership_unlock(lock_fd);
		if (status == 0 && saved_errno != 0)
			status = saved_errno;
	}
	return status;
}

/*
 * Teardown is driven by the persisted ownership manifest, not the current
 * DHCP option strings: a renewed lease may already have replaced those values.
 * Failed deletes remain recorded for a later retry; absent routes are complete.
 */
static inline int
dhcp_route_ownership_cleanup(
	const char *path,
	const char *owner,
	const char *ifname,
	int metric,
	dhcp_route_ownership_operation delete_operation,
	void *context)
{
	struct dhcp_route_ownership_entry entries[DHCP_ROUTE_OWNERSHIP_MAX_RECORDS];
	struct dhcp_route_ownership_entry remaining[DHCP_ROUTE_OWNERSHIP_MAX_RECORDS];
	char *lock_path;
	size_t count = 0, kept = 0, i, path_length;
	int lock_fd = -1, status, delete_status, saved_errno;

	if (!path || !path[0] ||
	    !dhcp_route_ownership_owner_is_valid(owner) ||
	    !dhcp_route_ownership_ifname_is_valid(ifname) ||
	    metric < 0 || metric >= INT_MAX || !delete_operation)
		return EINVAL;
	path_length = strlen(path);
	if (path_length > (size_t)-1 - sizeof(".lock"))
		return EOVERFLOW;
	lock_path = (char *)malloc(path_length + sizeof(".lock"));
	if (!lock_path)
		return ENOMEM;
	(void)snprintf(lock_path, path_length + sizeof(".lock"), "%s.lock", path);
	lock_fd = dhcp_route_ownership_lock(lock_path);
	free(lock_path);
	if (lock_fd < 0)
		return errno ? errno : EIO;

	status = dhcp_route_ownership_load(path, entries,
					   DHCP_ROUTE_OWNERSHIP_MAX_RECORDS,
					   &count);
	if (status != 0)
		goto out;
	status = 0;
	for (i = 0; i < count; ++i) {
		if (strcmp(entries[i].owner, owner) != 0 ||
		    strcmp(entries[i].ifname, ifname) != 0 ||
		    entries[i].metric != metric) {
			remaining[kept++] = entries[i];
			continue;
		}
		delete_status = delete_operation(context, &entries[i]);
		if (delete_status == 0 || delete_status == ESRCH ||
		    delete_status == ENOENT)
			continue;
		if (status == 0)
			status = delete_status;
		remaining[kept++] = entries[i];
	}
	delete_status = dhcp_route_ownership_save(path, remaining, kept);
	if (delete_status != 0 && status == 0)
		status = delete_status;

out:
	if (lock_fd >= 0) {
		saved_errno = dhcp_route_ownership_unlock(lock_fd);
		if (status == 0 && saved_errno != 0)
			status = saved_errno;
	}
	return status;
}

#endif /* PADAVAN_DHCP_ROUTE_OWNERSHIP_H */

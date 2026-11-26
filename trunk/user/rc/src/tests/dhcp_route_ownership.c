#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dhcp_route_ownership.h"

struct fake_routes {
	int add_status;
	int delete_status;
	unsigned int add_count;
	unsigned int delete_count;
};

static int
fake_add(void *opaque, const struct dhcp_route_ownership_entry *entry)
{
	struct fake_routes *fake = (struct fake_routes *)opaque;

	assert(dhcp_route_ownership_entry_is_valid(entry));
	fake->add_count++;
	return fake->add_status;
}

static int
fake_delete(void *opaque, const struct dhcp_route_ownership_entry *entry)
{
	struct fake_routes *fake = (struct fake_routes *)opaque;

	assert(dhcp_route_ownership_entry_is_valid(entry));
	fake->delete_count++;
	return fake->delete_status;
}

static void
make_entry(struct dhcp_route_ownership_entry *entry,
	   const char *owner, const char *ifname, int metric, const char *destination,
	   const char *gateway, const char *netmask)
{
	memset(entry, 0, sizeof(*entry));
	strncpy(entry->ifname, ifname, sizeof(entry->ifname) - 1);
	strncpy(entry->owner, owner, sizeof(entry->owner) - 1);
	entry->metric = metric;
	strncpy(entry->destination, destination, sizeof(entry->destination) - 1);
	strncpy(entry->gateway, gateway, sizeof(entry->gateway) - 1);
	strncpy(entry->netmask, netmask, sizeof(entry->netmask) - 1);
}

static void
write_metric_manifest(const char *path, const char *metric)
{
	FILE *stream = fopen(path, "w");

	assert(stream != NULL);
	assert(fprintf(stream, "%s\neth2 wan0_ %s 198.51.100.0 192.0.2.1 255.255.255.0\n",
		       DHCP_ROUTE_OWNERSHIP_MAGIC, metric) > 0);
	assert(fclose(stream) == 0);
}

int
main(void)
{
	char directory[] = "/tmp/dhcp-route-ownership.XXXXXX";
	char path[320], victim[320], link_path[320];
	char lock_state[320], lock_state_lock[340];
	struct dhcp_route_ownership_entry route, other, stale_route, malformed;
	char unterminated_ifname[IFNAMSIZ], unterminated_owner[16];
	struct fake_routes fake;
	struct stat st;
	int fd;
	unsigned int add_before;
	unsigned int delete_before;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(path, sizeof(path), "%s/owned.routes", directory) > 0);
	assert(snprintf(victim, sizeof(victim), "%s/victim", directory) > 0);
	assert(snprintf(link_path, sizeof(link_path), "%s/routes-link", directory) > 0);
	make_entry(&route, "wan0_", "eth2", 0, "198.51.100.0", "192.0.2.1", "255.255.255.0");
	make_entry(&other, "manv_", "eth2", 0, "203.0.113.0", "192.0.2.2", "255.255.255.0");
	make_entry(&stale_route, "wan0_", "ppp0", 7, "198.51.100.0", "192.0.2.3", "255.255.255.0");

	/* Fixed-size fields without a terminator must fail validation without over-read. */
	memset(unterminated_ifname, 'e', sizeof(unterminated_ifname));
	memset(unterminated_owner, 'x', sizeof(unterminated_owner));
	assert(!dhcp_route_ownership_ifname_is_valid(unterminated_ifname));
	assert(!dhcp_route_ownership_owner_is_valid(unterminated_owner));
	malformed = route;
	memset(malformed.ifname, 'e', sizeof(malformed.ifname));
	assert(!dhcp_route_ownership_entry_is_valid(&malformed));
	malformed = route;
	memset(malformed.owner, 'x', sizeof(malformed.owner));
	assert(!dhcp_route_ownership_entry_is_valid(&malformed));
	malformed = route;
	memset(malformed.destination, '1', sizeof(malformed.destination));
	assert(!dhcp_route_ownership_entry_is_valid(&malformed));
	malformed = route;
	memset(malformed.gateway, '1', sizeof(malformed.gateway));
	assert(!dhcp_route_ownership_entry_is_valid(&malformed));
	malformed = route;
	memset(malformed.netmask, '1', sizeof(malformed.netmask));
	assert(!dhcp_route_ownership_entry_is_valid(&malformed));

	/* A route that predated DHCP (EEXIST) is never entered in the manifest. */
	memset(&fake, 0, sizeof(fake));
	fake.add_status = EEXIST;
	assert(dhcp_route_ownership_add(path, &route, fake_add, fake_delete, &fake) == 0);
	assert(fake.add_count == 1 && fake.delete_count == 0);
	assert(dhcp_route_ownership_cleanup(path, "wan0_",
					    fake_delete, &fake) == 0);
	assert(fake.delete_count == 0);

	/* A successful add survives callback/process boundaries in the manifest. */
	fake.add_status = 0;
	assert(dhcp_route_ownership_add(path, &route, fake_add, fake_delete, &fake) == 0);
	assert(fake.add_count == 2 && fake.delete_count == 0);
	assert(stat(path, &st) == 0 && (st.st_mode & 0777) == 0600);

	/* A second DHCP callback observes its record; EEXIST does not lose ownership. */
	fake.add_status = EEXIST;
	assert(dhcp_route_ownership_add(path, &route, fake_add, fake_delete, &fake) == 0);
	assert(fake.add_count == 3 && fake.delete_count == 0);

	/* A failed delete retains ownership for a later retry. */
	fake.delete_status = EIO;
	assert(dhcp_route_ownership_cleanup(path, "wan0_",
					    fake_delete, &fake) == EIO);
	assert(fake.delete_count == 1);
	fake.delete_status = 0;
	assert(dhcp_route_ownership_cleanup(path, "wan0_",
					    fake_delete, &fake) == 0);
	assert(fake.delete_count == 2);
	assert(dhcp_route_ownership_cleanup(path, "wan0_",
					    fake_delete, &fake) == 0);
	assert(fake.delete_count == 2);

	/* Service ownership survives changes to interface and metric. */
	fake.add_status = 0;
	assert(dhcp_route_ownership_add(path, &route, fake_add, fake_delete, &fake) == 0);
	assert(dhcp_route_ownership_add(path, &other, fake_add, fake_delete, &fake) == 0);
	assert(dhcp_route_ownership_add(path, &stale_route, fake_add, fake_delete, &fake) == 0);
	delete_before = fake.delete_count;
	assert(dhcp_route_ownership_cleanup(path, "wan0_",
					    fake_delete, &fake) == 0);
	assert(fake.delete_count == delete_before + 2U);
	/* A later callback reloads the manifest and retains the other service. */
	fake.add_status = EEXIST;
	assert(dhcp_route_ownership_add(path, &other, fake_add, fake_delete, &fake) == 0);
	assert(dhcp_route_ownership_cleanup(path, "manv_",
					    fake_delete, &fake) == 0);
	assert(fake.delete_count == delete_before + 3U);

	/* Owner prefixes are strict service identifiers. */
	assert(dhcp_route_ownership_owner_is_valid("wan0_"));
	assert(dhcp_route_ownership_owner_is_valid("manv_"));
	assert(!dhcp_route_ownership_owner_is_valid(""));
	assert(!dhcp_route_ownership_owner_is_valid("wan0"));
	assert(!dhcp_route_ownership_owner_is_valid("wan0-x_"));
	assert(!dhcp_route_ownership_owner_is_valid("abcdefghijklmnop_"));
	assert(dhcp_route_ownership_cleanup(path, "invalid",
					    fake_delete, &fake) == EINVAL);

	/* A symlinked lock file must fail closed before any route add operation. */
	assert(snprintf(lock_state, sizeof(lock_state), "%s/locked.routes", directory) > 0);
	assert(snprintf(lock_state_lock, sizeof(lock_state_lock), "%s.lock", lock_state) > 0);
	assert(symlink(victim, lock_state_lock) == 0);
	add_before = fake.add_count;
	assert(dhcp_route_ownership_add(lock_state, &route,
					fake_add, fake_delete, &fake) != 0);
	assert(fake.add_count == add_before);
	unlink(lock_state_lock);

	/* Malformed/symlink state fails closed without invoking route deletion. */
	fd = open(victim, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0);
	assert(write(fd, "do-not-touch", 12) == 12);
	assert(close(fd) == 0);
	assert(symlink(victim, link_path) == 0);
	fake.delete_count = 0;
	assert(dhcp_route_ownership_cleanup(link_path, "wan0_",
					    fake_delete, &fake) != 0);
	assert(fake.delete_count == 0);

	fd = open(path, O_WRONLY | O_TRUNC);
	assert(fd >= 0);
	assert(write(fd, "corrupt\n", 8) == 8);
	assert(close(fd) == 0);
	assert(dhcp_route_ownership_cleanup(path, "wan0_",
					    fake_delete, &fake) == EINVAL);
	assert(fake.delete_count == 0);

	/* Numeric route metrics must be range-checked before conversion to int. */
	write_metric_manifest(path, "2147483648");
	assert(dhcp_route_ownership_cleanup(path, "wan0_",
					    fake_delete, &fake) == EINVAL);
	assert(fake.delete_count == 0);
	write_metric_manifest(path, "2147483647");
	assert(dhcp_route_ownership_cleanup(path, "wan0_",
					    fake_delete, &fake) == EINVAL);
	assert(fake.delete_count == 0);
	write_metric_manifest(path, "999999999999999");
	assert(dhcp_route_ownership_cleanup(path, "wan0_",
					    fake_delete, &fake) == EINVAL);
	assert(fake.delete_count == 0);
	write_metric_manifest(path, "-1");
	assert(dhcp_route_ownership_cleanup(path, "wan0_",
					    fake_delete, &fake) == EINVAL);
	assert(fake.delete_count == 0);
	write_metric_manifest(path, "7junk");
	assert(dhcp_route_ownership_cleanup(path, "wan0_",
					    fake_delete, &fake) == EINVAL);
	assert(fake.delete_count == 0);

	unlink(path);
	unlink(victim);
	unlink(link_path);
	{
		char lock_path[340];
		assert(snprintf(lock_path, sizeof(lock_path), "%s.lock", path) > 0);
		unlink(lock_path);
	}
	rmdir(directory);
	return 0;
}

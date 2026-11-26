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

int
main(void)
{
	char directory[] = "/tmp/dhcp-route-ownership.XXXXXX";
	char path[320], victim[320], link_path[320];
	struct dhcp_route_ownership_entry route, other, stale_route;
	struct fake_routes fake;
	struct stat st;
	int fd;
	unsigned int delete_before;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(path, sizeof(path), "%s/owned.routes", directory) > 0);
	assert(snprintf(victim, sizeof(victim), "%s/victim", directory) > 0);
	assert(snprintf(link_path, sizeof(link_path), "%s/routes-link", directory) > 0);
	make_entry(&route, "wan0_", "eth2", 0, "198.51.100.0", "192.0.2.1", "255.255.255.0");
	make_entry(&other, "manv_", "eth2", 0, "203.0.113.0", "192.0.2.2", "255.255.255.0");
	make_entry(&stale_route, "wan0_", "ppp0", 7, "198.51.100.0", "192.0.2.3", "255.255.255.0");

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

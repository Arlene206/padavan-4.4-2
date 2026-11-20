#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "../miniupnpd_redirect_cleanup_policy.h"

#include "miniupnpd_redirect_cleanup_policy.h"

static void
test_cleanup_status_preserves_each_failure(void)
{
	assert(miniupnpd_merge_cleanup_status(0, 0) == 0);
	assert(miniupnpd_merge_cleanup_status(-1, 0) == -1);
	assert(miniupnpd_merge_cleanup_status(0, -1) == -1);
	assert(miniupnpd_merge_cleanup_status(-1, -1) == -1);
	assert(miniupnpd_merge_cleanup_status(7, 0) == 7);
}

static void
test_primary_redirect_removal_is_independent_of_peer_cleanup(void)
{
	assert(miniupnpd_primary_redirect_removed(0, 0, -1, -1, 0, 0) == 0);
	assert(miniupnpd_primary_redirect_removed(1, 0, -1, -1, 0, 0) == 1);
	assert(miniupnpd_primary_redirect_removed(1, 1, -1, -1, 0, 0) == 0);
	assert(miniupnpd_primary_redirect_removed(1, 1, 0, 0, 0, 0) == 1);
	assert(miniupnpd_primary_redirect_removed(1, 1, 0, -1, 1, 0) == 0);
	assert(miniupnpd_primary_redirect_removed(1, 1, 0, -1, 1, -1) == 1);
	assert(miniupnpd_primary_redirect_removed(1, 1, 0, -1, 0, 0) == 0);
}



static unsigned int metadata_cleanup_calls;
static unsigned short metadata_cleanup_eport;
static int metadata_cleanup_proto;

static void
record_metadata_cleanup(unsigned short eport, int proto)
{
	++metadata_cleanup_calls;
	metadata_cleanup_eport = eport;
	metadata_cleanup_proto = proto;
}

static void
test_metadata_cleanup_tracks_primary_mapping_and_preserves_status(void)
{
	metadata_cleanup_calls = 0;
	assert(miniupnpd_finalize_redirect_cleanup(
		-1, 1, record_metadata_cleanup, 4567, 6) == -1);
	assert(metadata_cleanup_calls == 1);
	assert(metadata_cleanup_eport == 4567);
	assert(metadata_cleanup_proto == 6);

	metadata_cleanup_calls = 0;
	assert(miniupnpd_finalize_redirect_cleanup(
		-1, 0, record_metadata_cleanup, 4567, 6) == -1);
	assert(metadata_cleanup_calls == 0);

	assert(miniupnpd_finalize_redirect_cleanup(
		0, 0, record_metadata_cleanup, 4567, 17) == 0);
	assert(metadata_cleanup_calls == 0);

	assert(miniupnpd_finalize_redirect_cleanup(
		0, 1, record_metadata_cleanup, 7654, 17) == 0);
	assert(metadata_cleanup_calls == 1);
	assert(metadata_cleanup_eport == 7654);
	assert(metadata_cleanup_proto == 17);

	metadata_cleanup_calls = 0;
	assert(miniupnpd_finalize_redirect_cleanup(
		-1, 1, NULL, 7654, 17) == -1);
	assert(metadata_cleanup_calls == 0);
}



static void
test_cleanup_helpers_restore_original_errno(void)
{
	errno = EBUSY;
	assert(miniupnpd_restore_errno_on_failure(0, EACCES) == 0);
	assert(errno == EBUSY);

	errno = 0;
	assert(miniupnpd_restore_errno_on_failure(-1, EACCES) == -1);
	assert(errno == EACCES);

	errno = 0;
	assert(miniupnpd_restore_errno_on_failure(-1, 0) == -1);
	assert(errno == EIO);
}

static void
test_peer_cleanup_requires_primary_mapping_removal(void)
{
	assert(miniupnpd_peer_redirect_cleanup_allowed(0) == 0);
	assert(miniupnpd_peer_redirect_cleanup_allowed(1) == 1);
}

int
main(void)
{
	test_cleanup_status_preserves_each_failure();
	test_primary_redirect_removal_is_independent_of_peer_cleanup();
	test_metadata_cleanup_tracks_primary_mapping_and_preserves_status();
	test_peer_cleanup_requires_primary_mapping_removal();
	test_cleanup_helpers_restore_original_errno();
	puts("MiniUPnP redirect cleanup status policy: PASS");
	return 0;
}

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "storage_home_link_policy.h"

int
main(void)
{
	assert(storage_home_link_restore_status(1, -1, EACCES, -1, EIO) == 0);
	assert(storage_home_link_restore_status(0, 0, 0, 0, 0) == 0);
	assert(storage_home_link_restore_status(0, -1, ENOENT, 0, 0) == 0);
	assert(storage_home_link_restore_status(0, -1, EACCES, 0, 0) == EACCES);
	assert(storage_home_link_restore_status(0, -1, 0, 0, 0) == EIO);
	assert(storage_home_link_restore_status(0, 0, 0, -1, ENOSPC) == ENOSPC);
	assert(storage_home_link_restore_status(0, 0, 0, -1, 0) == EIO);

	puts("storage home-directory link restoration status: PASS");
	return 0;
}

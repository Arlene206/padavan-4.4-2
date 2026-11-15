#include <stdio.h>
#include "storage_persistence_policy.h"
int
main(void)
{
	if (!storage_persistence_mtd_may_run(0) ||
	    storage_persistence_mtd_may_run(-1) ||
	    storage_persistence_mtd_may_run(1) ||
	    !storage_persistence_complete(0, 0) ||
	    storage_persistence_complete(-1, 0) ||
	    storage_persistence_complete(1, 0) ||
	    storage_persistence_complete(0, -1) ||
	    storage_persistence_complete(0, 1)) {
		fprintf(stderr, "storage persistence fail-closed gate regression: FAIL\n");
		return 1;
	}
	puts("storage persistence fail-closed gate regression: PASS");
	return 0;
}

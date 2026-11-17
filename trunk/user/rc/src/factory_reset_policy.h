#ifndef __FACTORY_RESET_POLICY_H__
#define __FACTORY_RESET_POLICY_H__

/* The persistent NVRAM reset marker is safe only after storage erase succeeds. */
static inline int
factory_reset_storage_allows_nvram(int storage_status)
{
	return storage_status == 0;
}

/* PID 1 may be asked to reboot only after both persistent reset steps succeed. */
static inline int
factory_reset_results_allow_reboot(int storage_status, int nvram_status)
{
	return storage_status == 0 && nvram_status == 0;
}

#endif

#ifndef HTTPD_NVRAM_COMMIT_POLICY_H
#define HTTPD_NVRAM_COMMIT_POLICY_H

/* nvram_manual defers persistence until an explicit commit or reboot. */
#define HTTPD_NVRAM_COMMIT_DEFERRED 1

static inline int
httpd_nvram_commit_clears_dirty(int commit_result)
{
	return commit_result == 0;
}

static inline int
httpd_nvram_reboot_requires_commit(int manual_mode, int nvram_modified)
{
	return manual_mode || nvram_modified;
}

static inline int
httpd_nvram_reboot_is_safe(int commit_required, int commit_result)
{
	return !commit_required || commit_result == 0;
}

#endif /* HTTPD_NVRAM_COMMIT_POLICY_H */

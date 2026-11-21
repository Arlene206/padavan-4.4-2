/* Gate NVRAM profile restore reboot on durable commit completion. */
#ifndef HTTPD_NVRAM_RESTORE_POLICY_H
#define HTTPD_NVRAM_RESTORE_POLICY_H

static int
httpd_nvram_restore_is_committed(int restore_result, int commit_result)
{
	return (restore_result == 0 && commit_result == 0);
}

#endif /* HTTPD_NVRAM_RESTORE_POLICY_H */

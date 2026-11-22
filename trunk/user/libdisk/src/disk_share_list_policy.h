#ifndef PADAVAN_DISK_SHARE_LIST_POLICY_H
#define PADAVAN_DISK_SHARE_LIST_POLICY_H

/*
 * Reset caller-owned outputs before rejecting invalid inputs. This keeps
 * error cleanup safe in callers that always release the returned list.
 */
static inline int
disk_share_list_prepare_outputs(const char *mount_path, int *sh_num,
                                char ***folder_list)
{
	if (sh_num)
		*sh_num = 0;
	if (folder_list)
		*folder_list = NULL;

	if (!mount_path || !*mount_path || !sh_num || !folder_list)
		return -1;

	return 0;
}

#endif /* PADAVAN_DISK_SHARE_LIST_POLICY_H */

#ifndef VSF_FTP_LISTING_POLICY_H
#define VSF_FTP_LISTING_POLICY_H

/*
 * LIST -R is client-controlled and retains a directory descriptor and a
 * subdirectory-name list at each recursion level. Bound both recursion and
 * collected entries so a large/deep tree cannot exhaust the FTP child.
 */
#define VSF_LISTING_MAX_DEPTH 32U
#define VSF_LISTING_MAX_ENTRIES_PER_DIR 4096U
#define VSF_LISTING_MAX_ENTRIES_TOTAL 32768U

#define vsf_listing_entry_limit_reached(entries_in_dir, entries_total) \
  ((entries_in_dir) >= VSF_LISTING_MAX_ENTRIES_PER_DIR || \
   (entries_total) >= VSF_LISTING_MAX_ENTRIES_TOTAL)

#define vsf_listing_depth_limit_reached(depth, child_directories) \
  ((depth) >= VSF_LISTING_MAX_DEPTH && (child_directories) != 0)

#endif /* VSF_FTP_LISTING_POLICY_H */

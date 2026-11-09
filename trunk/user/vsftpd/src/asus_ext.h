#ifndef ASUS_EXT_H
#define ASUS_EXT_H

#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

struct mystr;
struct passwd;
struct vsf_session;

#define PERM_READ 1
#define PERM_WRITE 2
#define PERM_DELETE 3

int asus_share_mode_read(void);
int asus_check_permission(struct vsf_session* p_sess, int perm);
int asus_check_permission_path(struct vsf_session* p_sess, int perm,
                               const struct mystr* p_path_str);
int asus_check_permission_existing_path(struct vsf_session* p_sess, int perm,
                                        const struct mystr* p_path_str);
int asus_check_permission_upload(struct vsf_session* p_sess,
                                 const struct mystr* p_path_str);
int asus_make_directory_path(struct vsf_session* p_sess,
                            const struct mystr* p_path_str,
                            unsigned int mode, int* p_permission_denied);
int asus_remove_path(struct vsf_session* p_sess,
                     const struct mystr* p_path_str, int remove_directory,
                     int* p_permission_denied);
int asus_open_existing_path(struct vsf_session* p_sess, int perm,
                             const struct mystr* p_path_str, int stat_only,
                             int* p_permission_denied);
int asus_open_upload_path(struct vsf_session* p_sess,
                          const struct mystr* p_path_str, int exclusive,
                          unsigned int mode, int* p_parent_fd,
                          char* p_leaf, unsigned int leaf_size,
                          int* p_cleanup_allowed);
int asus_cleanup_failed_upload(int parent_fd, const char* leaf, int file_fd);
int asus_check_permission_directory(struct vsf_session* p_sess,
                                    const struct mystr* p_path_str);
int asus_check_file_visible(struct vsf_session* p_sess, const struct mystr* p_filename_str);
int asus_check_auth(struct mystr* p_user_str, const struct mystr* p_pass_str);
struct passwd *asus_getpwnam(const char *name);


#endif /* ASUS_EXT_H */


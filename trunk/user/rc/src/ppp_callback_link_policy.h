#ifndef PADAVAN_PPP_CALLBACK_LINK_POLICY_H
#define PADAVAN_PPP_CALLBACK_LINK_POLICY_H
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
struct ppp_callback_link_ops {
 int (*lstat_fn)(const char *,struct stat *);
 ssize_t (*readlink_fn)(const char *,char *,size_t);
 int (*symlink_fn)(const char *,const char *);
 int (*unlink_fn)(const char *);
 uid_t (*geteuid_fn)(void);
};
static const struct ppp_callback_link_ops ppp_callback_link_system_ops={lstat,readlink,symlink,unlink,geteuid};
static inline int
ppp_callback_link_directory_ready_with(const char *path,const struct ppp_callback_link_ops *ops)
{
 struct stat st;
 if(!path||!path[0]||!ops||!ops->lstat_fn||!ops->geteuid_fn){errno=EINVAL;return -1;}
 if(ops->lstat_fn(path,&st)!=0)return -1;
 if(!S_ISDIR(st.st_mode)||st.st_uid!=ops->geteuid_fn()||
    ((st.st_mode&(S_IWGRP|S_IWOTH))&&!(st.st_mode&S_ISVTX))){errno=EPERM;return -1;}
 return 0;
}
static inline int ppp_callback_link_directory_ready(const char *path)
{return ppp_callback_link_directory_ready_with(path,&ppp_callback_link_system_ops);}
/* 1=exact owned symlink, 0=absent, -1=conflict/system error. */
static inline int
ppp_callback_link_state_with(const char *target,const char *path,const struct ppp_callback_link_ops *ops)
{
 struct stat st;char actual[PATH_MAX];ssize_t length;
 if(!target||!target[0]||!path||!path[0]||!ops||!ops->lstat_fn||!ops->readlink_fn||!ops->geteuid_fn){errno=EINVAL;return -1;}
 if(ops->lstat_fn(path,&st)!=0){if(errno==ENOENT)return 0;return -1;}
 if(!S_ISLNK(st.st_mode)){errno=EEXIST;return -1;}
 if(st.st_uid!=ops->geteuid_fn()){errno=EPERM;return -1;}
 length=ops->readlink_fn(path,actual,sizeof(actual));if(length<0)return -1;
 if((size_t)length>=sizeof(actual)){errno=ENAMETOOLONG;return -1;}
 actual[length]='\0';if(strcmp(actual,target)!=0){errno=EEXIST;return -1;}
 return 1;
}
static inline int
ppp_callback_link_ensure_with(const char *target,const char *path,unsigned char *created,const struct ppp_callback_link_ops *ops)
{
 int state,saved_errno;
 if(!created||!ops||!ops->symlink_fn){errno=EINVAL;return -1;}
 *created=0;state=ppp_callback_link_state_with(target,path,ops);
 if(state<0)
  return -1;
 if(state>0)
  return 0;
 if(ops->symlink_fn(target,path)==0){*created=1;return 0;}
 saved_errno=errno?errno:EIO;
 if(saved_errno==EEXIST){state=ppp_callback_link_state_with(target,path,ops);if(state>0)return 0;if(state<0)return -1;}
 errno=saved_errno;return -1;
}
static inline int ppp_callback_link_ensure(const char *target,const char *path,unsigned char *created)
{return ppp_callback_link_ensure_with(target,path,created,&ppp_callback_link_system_ops);}
static inline int
ppp_callback_link_remove_with(const char *target,const char *path,const struct ppp_callback_link_ops *ops)
{
 int state;if(!ops||!ops->unlink_fn){errno=EINVAL;return -1;}
 state=ppp_callback_link_state_with(target,path,ops);if(state<=0)return state;
 if(ops->unlink_fn(path)==0||errno==ENOENT)
  return 0;
 return -1;
}
static inline int ppp_callback_link_remove(const char *target,const char *path)
{return ppp_callback_link_remove_with(target,path,&ppp_callback_link_system_ops);}

/*
 * Remove a callback only when it is a direct child of the expected private
 * directory and is still the exact symlink owned by this effective uid.
 * A missing directory/link is already clean; unsafe objects are preserved.
 */
static inline int
ppp_callback_link_remove_from_directory_with(
    const char *directory, const char *target, const char *path,
    const struct ppp_callback_link_ops *ops)
{
    struct stat directory_stat;
    const char *basename;
    size_t directory_length;
    int state;

    if (!directory || !directory[0] || !target || !target[0] ||
        !path || !path[0] || !ops || !ops->lstat_fn ||
        !ops->readlink_fn || !ops->unlink_fn || !ops->geteuid_fn) {
        errno = EINVAL;
        return -1;
    }
    directory_length = strlen(directory);
    while (directory_length > 1 && directory[directory_length - 1] == '/')
        directory_length--;
    if (directory_length == 1 && directory[0] == '/') {
        if (path[0] != '/' || path[1] == '\0') {
            errno = EINVAL;
            return -1;
        }
        basename = path + 1;
    } else {
        if (strncmp(directory, path, directory_length) != 0 ||
            path[directory_length] != '/') {
            errno = EINVAL;
            return -1;
        }
        basename = path + directory_length + 1;
    }
    if (!basename[0] || strchr(basename, '/') ||
        strcmp(basename, ".") == 0 || strcmp(basename, "..") == 0) {
        errno = EINVAL;
        return -1;
    }
    if (ops->lstat_fn(directory, &directory_stat) != 0)
        return errno == ENOENT ? 0 : -1;
    if (!S_ISDIR(directory_stat.st_mode) ||
        directory_stat.st_uid != ops->geteuid_fn() ||
        ((directory_stat.st_mode & (S_IWGRP | S_IWOTH)) &&
         !(directory_stat.st_mode & S_ISVTX))) {
        errno = EPERM;
        return -1;
    }
    state = ppp_callback_link_state_with(target, path, ops);
    if (state <= 0)
        return state;
    if (ops->unlink_fn(path) == 0 || errno == ENOENT)
        return 0;
    return -1;
}
static inline int
ppp_callback_link_remove_from_directory(const char *directory,
                                        const char *target,
                                        const char *path)
{
    return ppp_callback_link_remove_from_directory_with(
        directory, target, path, &ppp_callback_link_system_ops);
}

#endif

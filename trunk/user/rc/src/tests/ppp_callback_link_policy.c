#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ppp_callback_link_policy.h"
static uid_t different_euid(void){return (uid_t)(geteuid()+1);}
static int fail_symlink(const char*t,const char*p){(void)t;(void)p;errno=EACCES;return -1;}
static int race_symlink(const char*t,const char*p){if(symlink(t,p)!=0)return -1;errno=EEXIST;return -1;}
static void test_shared_directory_requires_sticky_write_protection(void){
 char d[]="/tmp/ppp-callback-dir.XXXXXX";assert(mkdtemp(d));
 assert(ppp_callback_link_directory_ready(d)==0);assert(chmod(d,0777)==0);
 errno=0;assert(ppp_callback_link_directory_ready(d)==-1&&errno==EPERM);
 assert(chmod(d,01777)==0&&ppp_callback_link_directory_ready(d)==0);
 assert(chmod(d,0700)==0&&rmdir(d)==0);
}
static void test_create_existing_and_remove(void){
 char d[]="/tmp/ppp-callback-link.XXXXXX",p[PATH_MAX];unsigned char made=0;struct stat st;
 assert(mkdtemp(d));assert(snprintf(p,sizeof(p),"%s/callback",d)>0);
 assert(ppp_callback_link_ensure("/sbin/rc",p,&made)==0&&made==1);
 assert(lstat(p,&st)==0&&S_ISLNK(st.st_mode));made=1;
 assert(ppp_callback_link_ensure("/sbin/rc",p,&made)==0&&made==0);
 assert(ppp_callback_link_remove("/sbin/rc",p)==0);
 assert(lstat(p,&st)!=0&&errno==ENOENT&&ppp_callback_link_remove("/sbin/rc",p)==0);
 assert(rmdir(d)==0);
}
static void test_regular_file_collision_is_preserved(void){
 char d[]="/tmp/ppp-callback-file.XXXXXX",p[PATH_MAX],buf[8]={0};unsigned char made=0;int fd;
 assert(mkdtemp(d));assert(snprintf(p,sizeof(p),"%s/callback",d)>0);
 fd=open(p,O_CREAT|O_EXCL|O_WRONLY,0600);assert(fd>=0&&write(fd,"keep",4)==4&&close(fd)==0);
 errno=0;assert(ppp_callback_link_ensure("/sbin/rc",p,&made)==-1&&errno==EEXIST&&made==0);
 errno=0;assert(ppp_callback_link_remove("/sbin/rc",p)==-1&&errno==EEXIST);
 fd=open(p,O_RDONLY);assert(fd>=0&&read(fd,buf,4)==4&&close(fd)==0);
 assert(memcmp(buf,"keep",4)==0&&unlink(p)==0&&rmdir(d)==0);
}
static void test_wrong_symlink_is_preserved(void){
 char d[]="/tmp/ppp-callback-wrong.XXXXXX",p[PATH_MAX],t[PATH_MAX];unsigned char made=0;ssize_t n;
 assert(mkdtemp(d));assert(snprintf(p,sizeof(p),"%s/callback",d)>0&&symlink("/tmp/not-rc",p)==0);
 errno=0;assert(ppp_callback_link_ensure("/sbin/rc",p,&made)==-1&&errno==EEXIST&&made==0);
 errno=0;assert(ppp_callback_link_remove("/sbin/rc",p)==-1&&errno==EEXIST);
 n=readlink(p,t,sizeof(t)-1);assert(n==(ssize_t)strlen("/tmp/not-rc"));t[n]='\0';
 assert(strcmp(t,"/tmp/not-rc")==0&&unlink(p)==0&&rmdir(d)==0);
}
static void test_non_owner_exact_symlink_is_preserved(void){
 char d[]="/tmp/ppp-callback-owner.XXXXXX",p[PATH_MAX],t[PATH_MAX];unsigned char made=0;ssize_t n;
 struct ppp_callback_link_ops ops={lstat,readlink,symlink,unlink,different_euid};
 assert(mkdtemp(d));assert(snprintf(p,sizeof(p),"%s/callback",d)>0&&symlink("/sbin/rc",p)==0);
 errno=0;assert(ppp_callback_link_ensure_with("/sbin/rc",p,&made,&ops)==-1&&errno==EPERM&&made==0);
 errno=0;assert(ppp_callback_link_remove_with("/sbin/rc",p,&ops)==-1&&errno==EPERM);
 n=readlink(p,t,sizeof(t)-1);assert(n==(ssize_t)strlen("/sbin/rc"));t[n]='\0';
 assert(strcmp(t,"/sbin/rc")==0&&unlink(p)==0&&rmdir(d)==0);
}
static void test_create_error_is_preserved(void){
 char d[]="/tmp/ppp-callback-error.XXXXXX",p[PATH_MAX];unsigned char made=1;
 struct ppp_callback_link_ops ops={lstat,readlink,fail_symlink,unlink,geteuid};
 assert(mkdtemp(d));assert(snprintf(p,sizeof(p),"%s/callback",d)>0);errno=0;
 assert(ppp_callback_link_ensure_with("/sbin/rc",p,&made,&ops)==-1&&errno==EACCES&&made==0);
 assert(rmdir(d)==0);
}
static void test_eexist_race_accepts_only_exact_link(void){
 char d[]="/tmp/ppp-callback-race.XXXXXX",p[PATH_MAX];unsigned char made=1;
 struct ppp_callback_link_ops ops={lstat,readlink,race_symlink,unlink,geteuid};
 assert(mkdtemp(d));assert(snprintf(p,sizeof(p),"%s/callback",d)>0);
 assert(ppp_callback_link_ensure_with("/sbin/rc",p,&made,&ops)==0&&made==0);
 assert(ppp_callback_link_remove("/sbin/rc",p)==0&&rmdir(d)==0);
}

static int fail_unlink(const char *path)
{ (void)path; errno=EIO; return -1; }
static void test_checked_remove_rejects_unsafe_directory(void)
{
 char d[]="/tmp/ppp-callback-unsafe.XXXXXX",p[PATH_MAX]; struct stat st;
 assert(mkdtemp(d)); assert(snprintf(p,sizeof(p),"%s/callback",d)>0);
 assert(symlink("/sbin/rc",p)==0 && chmod(d,0777)==0);
 errno=0;
 assert(ppp_callback_link_remove_from_directory(d,"/sbin/rc",p)==-1&&errno==EPERM);
 assert(lstat(p,&st)==0&&S_ISLNK(st.st_mode));
 assert(chmod(d,0700)==0);
 assert(ppp_callback_link_remove_from_directory(d,"/sbin/rc",p)==0&&rmdir(d)==0);
}
static void test_checked_remove_rejects_directory_symlink(void)
{
 char root[]="/tmp/ppp-callback-parent.XXXXXX",real[PATH_MAX],alias[PATH_MAX];
 char p[PATH_MAX],alias_path[PATH_MAX],actual[PATH_MAX]; ssize_t n; struct stat st;
 assert(mkdtemp(root)); assert(snprintf(real,sizeof(real),"%s/real",root)>0&&mkdir(real,0700)==0);
 assert(snprintf(p,sizeof(p),"%s/callback",real)>0&&symlink("/sbin/rc",p)==0);
 assert(snprintf(alias,sizeof(alias),"%s/alias",root)>0&&symlink(real,alias)==0);
 assert(snprintf(alias_path,sizeof(alias_path),"%s/callback",alias)>0);
 errno=0;
 assert(ppp_callback_link_remove_from_directory(alias,"/sbin/rc",alias_path)==-1&&errno==EPERM);
 n=readlink(p,actual,sizeof(actual)-1); assert(n==(ssize_t)strlen("/sbin/rc"));
 actual[n]='\0'; assert(strcmp(actual,"/sbin/rc")==0&&lstat(p,&st)==0&&S_ISLNK(st.st_mode));
 assert(unlink(p)==0&&unlink(alias)==0&&rmdir(real)==0&&rmdir(root)==0);
}
static void test_checked_remove_rejects_non_child_path(void)
{
 char d[]="/tmp/ppp-callback-child.XXXXXX",p[PATH_MAX],with_slash[PATH_MAX];
 assert(mkdtemp(d));
 assert(snprintf(p,sizeof(p),"%s/callback",d)>0);
 assert(snprintf(with_slash,sizeof(with_slash),"%s/",d)>0); errno=0;
 assert(ppp_callback_link_remove_from_directory(with_slash,"/sbin/rc",p)==-1&&errno==EINVAL);
 assert(snprintf(p,sizeof(p),"%s/sub/callback",d)>0); errno=0;
 assert(ppp_callback_link_remove_from_directory(d,"/sbin/rc",p)==-1&&errno==EINVAL);
 assert(rmdir(d)==0);
}
static void test_checked_remove_preserves_unlink_errno_and_object(void)
{
 char d[]="/tmp/ppp-callback-unlink.XXXXXX",p[PATH_MAX];
 struct ppp_callback_link_ops ops={lstat,readlink,symlink,fail_unlink,geteuid}; struct stat st;
 assert(mkdtemp(d)); assert(snprintf(p,sizeof(p),"%s/callback",d)>0&&symlink("/sbin/rc",p)==0);
 errno=0;
 assert(ppp_callback_link_remove_from_directory_with(d,"/sbin/rc",p,&ops)==-1&&errno==EIO);
 assert(lstat(p,&st)==0&&S_ISLNK(st.st_mode));
 assert(unlink(p)==0&&rmdir(d)==0);
}
static void test_checked_remove_treats_missing_directory_as_clean(void)
{
 char d[]="/tmp/ppp-callback-missing.XXXXXX",p[PATH_MAX]; assert(mkdtemp(d));
 assert(snprintf(p,sizeof(p),"%s/callback",d)>0&&rmdir(d)==0);
 assert(ppp_callback_link_remove_from_directory(d,"/sbin/rc",p)==0);
}

int main(void){
 unsigned char made=1;
 test_shared_directory_requires_sticky_write_protection();test_create_existing_and_remove();
 test_regular_file_collision_is_preserved();test_wrong_symlink_is_preserved();
 test_non_owner_exact_symlink_is_preserved();test_create_error_is_preserved();
 test_eexist_race_accepts_only_exact_link();
 test_checked_remove_rejects_unsafe_directory();
 test_checked_remove_rejects_directory_symlink();
 test_checked_remove_rejects_non_child_path();
 test_checked_remove_preserves_unlink_errno_and_object();
 test_checked_remove_treats_missing_directory_as_clean();
 errno=0;
 assert(ppp_callback_link_ensure(NULL,"/tmp/no-link",&made)==-1&&errno==EINVAL&&made==0);
 puts("PPP callback link ownership policy: PASS");return 0;
}

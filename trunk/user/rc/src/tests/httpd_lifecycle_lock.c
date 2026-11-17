#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "httpd_lifecycle_lock.h"
static void put_byte(int fd,char c){ssize_t n;do{n=write(fd,&c,1);}while(n<0&&errno==EINTR);assert(n==1);}
static char get_byte(int fd){char c;ssize_t n;do{n=read(fd,&c,1);}while(n<0&&errno==EINTR);assert(n==1);return c;}
int main(void)
{
	char dir[]="/tmp/httpd-lock-test.XXXXXX";
	char path[256],hard[256],sym[256],target[256];
	struct stat a,b;
	int p2c[2],c2p[2],fd,cfd,status;
	pid_t child;
	assert(httpd_lifecycle_lock_acquire_path(NULL,0)==-1&&errno==EINVAL);
	assert(httpd_lifecycle_lock_acquire_path("",0)==-1&&errno==EINVAL);
	assert(httpd_lifecycle_lock_release(-1)==-1&&errno==EBADF);
	assert(mkdtemp(dir)!=NULL);
	assert(snprintf(path,sizeof(path),"%s/lock",dir)>0);
	assert(snprintf(hard,sizeof(hard),"%s/hard",dir)>0);
	assert(snprintf(sym,sizeof(sym),"%s/sym",dir)>0);
	assert(snprintf(target,sizeof(target),"%s/target",dir)>0);
	fd=httpd_lifecycle_lock_acquire_path(path,0);
	assert(fd>=0&&fstat(fd,&a)==0);
	assert(S_ISREG(a.st_mode)&&a.st_uid==geteuid()&&a.st_nlink==1);
	assert((a.st_mode&0777)==0600);
	assert(httpd_lifecycle_lock_release(fd)==0&&stat(path,&b)==0);
	assert(a.st_dev==b.st_dev&&a.st_ino==b.st_ino);
	assert(pipe(p2c)==0&&pipe(c2p)==0);
	child=fork();assert(child>=0);
	if(child==0){
		close(p2c[1]);close(c2p[0]);
		cfd=httpd_lifecycle_lock_acquire_path(path,0);
		if(cfd<0)_exit(10);
		put_byte(c2p[1],'L');
		if(get_byte(p2c[0])!='R')_exit(11);
		if(httpd_lifecycle_lock_release(cfd)!=0)_exit(12);
		close(p2c[0]);close(c2p[1]);_exit(0);
	}
	close(p2c[0]);close(c2p[1]);
	assert(get_byte(c2p[0])=='L');
	assert(httpd_lifecycle_lock_acquire_path(path,0)==-1&&errno==EBUSY);
	put_byte(p2c[1],'R');close(p2c[1]);
	assert(waitpid(child,&status,0)==child&&WIFEXITED(status)&&WEXITSTATUS(status)==0);
	close(c2p[0]);
	fd=httpd_lifecycle_lock_acquire_path(path,0);
	assert(fd>=0&&httpd_lifecycle_lock_release(fd)==0);
	assert(chmod(path,0644)==0);
	assert(httpd_lifecycle_lock_acquire_path(path,0)==-1&&errno==EPERM);
	assert(chmod(path,0600)==0&&link(path,hard)==0);
	assert(httpd_lifecycle_lock_acquire_path(path,0)==-1&&errno==EPERM);
	assert(unlink(hard)==0);
	cfd=open(target,O_WRONLY|O_CREAT|O_EXCL,0600);assert(cfd>=0&&close(cfd)==0);
	assert(symlink(target,sym)==0);
	assert(httpd_lifecycle_lock_acquire_path(sym,0)==-1&&errno==ELOOP);
	assert(unlink(sym)==0&&unlink(target)==0);
	assert(httpd_lifecycle_lock_acquire_path(dir,0)==-1);
	assert(unlink(path)==0&&rmdir(dir)==0);
	puts("HTTPD cross-process lifecycle lock/path integrity: PASS");
	return 0;
}

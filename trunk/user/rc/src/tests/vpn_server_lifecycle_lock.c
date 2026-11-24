#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "vpn_server_lifecycle_lock.h"

static void write_byte(int fd, char byte)
{
	ssize_t n;
	do { n = write(fd, &byte, 1); } while (n < 0 && errno == EINTR);
	assert(n == 1);
}
static char read_byte(int fd)
{
	char byte;
	ssize_t n;
	do { n = read(fd, &byte, 1); } while (n < 0 && errno == EINTR);
	assert(n == 1);
	return byte;
}
int main(void)
{
	char dir[] = "/tmp/vpns-lock-test.XXXXXX";
	char path[256], hard[256], sym[256], target[256];
	struct stat before, after;
	int p2c[2], c2p[2], lockfd, childfd, status;
	pid_t child;

	assert(vpn_server_lifecycle_lock_acquire_path(NULL, 0) == -1 && errno == EINVAL);
	assert(vpn_server_lifecycle_lock_acquire_path("", 0) == -1 && errno == EINVAL);
	assert(vpn_server_lifecycle_lock_release(-1) == -1 && errno == EBADF);
	assert(mkdtemp(dir) != NULL);
	assert(snprintf(path,sizeof(path),"%s/lock",dir)>0);
	assert(snprintf(hard,sizeof(hard),"%s/hard",dir)>0);
	assert(snprintf(sym,sizeof(sym),"%s/sym",dir)>0);
	assert(snprintf(target,sizeof(target),"%s/target",dir)>0);

	lockfd=vpn_server_lifecycle_lock_acquire_path(path,0);
	assert(lockfd>=0 && fstat(lockfd,&before)==0);
	assert(S_ISREG(before.st_mode) && before.st_uid==geteuid());
	assert(before.st_nlink==1 && (before.st_mode&0777)==0600);
	assert(vpn_server_lifecycle_lock_release(lockfd)==0);
	assert(stat(path,&after)==0 && before.st_dev==after.st_dev && before.st_ino==after.st_ino);

	assert(pipe(p2c)==0 && pipe(c2p)==0);
	child=fork();
	assert(child>=0);
	if(child==0) {
		close(p2c[1]); close(c2p[0]);
		childfd=vpn_server_lifecycle_lock_acquire_path(path,0);
		if(childfd<0) _exit(10);
		write_byte(c2p[1],'L');
		if(read_byte(p2c[0])!='R') _exit(11);
		if(vpn_server_lifecycle_lock_release(childfd)!=0) _exit(12);
		_exit(0);
	}
	close(p2c[0]); close(c2p[1]);
	assert(read_byte(c2p[0])=='L');
	assert(vpn_server_lifecycle_lock_acquire_path(path,0)==-1 && errno==EBUSY);
	write_byte(p2c[1],'R'); close(p2c[1]);
	assert(waitpid(child,&status,0)==child && WIFEXITED(status) && WEXITSTATUS(status)==0);
	lockfd=vpn_server_lifecycle_lock_acquire_path(path,0);
	assert(lockfd>=0 && vpn_server_lifecycle_lock_release(lockfd)==0);
	close(c2p[0]);

	assert(chmod(path,0644)==0);
	assert(vpn_server_lifecycle_lock_acquire_path(path,0)==-1 && errno==EPERM);
	assert(chmod(path,0600)==0);
	assert(link(path,hard)==0);
	assert(vpn_server_lifecycle_lock_acquire_path(path,0)==-1 && errno==EPERM);
	assert(unlink(hard)==0);
	childfd=open(target,O_WRONLY|O_CREAT|O_EXCL,0600);
	assert(childfd>=0 && close(childfd)==0);
	assert(unlink(path)==0 && symlink(target,sym)==0);
	assert(vpn_server_lifecycle_lock_acquire_path(sym,0)==-1 && errno==ELOOP);
	assert(unlink(sym)==0 && unlink(target)==0);
	assert(vpn_server_lifecycle_lock_acquire_path(dir,0)==-1);
	assert(rmdir(dir)==0);
	puts("VPN server lifecycle lock policy: PASS");
	return 0;
}

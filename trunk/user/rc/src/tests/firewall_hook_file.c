#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../firewall_hook_file.h"
static void write_text(const char *p,const char *v){FILE *f=fopen(p,"w");assert(f);assert(fputs(v,f)>=0);assert(fclose(f)==0);}

static int fail_unlink_count;

static int
test_unlink_once(const char *path)
{
	if (fail_unlink_count > 0) {
		--fail_unlink_count;
		errno = EACCES;
		return -1;
	}
	return unlink(path);
}
int main(void){
 char root[]="/tmp/firewall-hook-file-test.XXXXXX",dir[256],src[256],linkp[256],hard[256],fifo[256],sentinel[256],staged[512],buf[128];
 FILE *fp;struct stat st;int fd;
 assert(mkdtemp(root));
 assert(snprintf(dir,sizeof(dir),"%s/private",root)>0);assert(mkdir(dir,0700)==0);
 assert(snprintf(src,sizeof(src),"%s/source.sh",root)>0);
 assert(snprintf(linkp,sizeof(linkp),"%s/source-link.sh",root)>0);
 assert(snprintf(hard,sizeof(hard),"%s/source-hardlink.sh",root)>0);
 assert(snprintf(fifo,sizeof(fifo),"%s/source.fifo",root)>0);
 assert(snprintf(sentinel,sizeof(sentinel),"%s/sentinel",root)>0);
 write_text(src,"#!/bin/sh\nprintf 'trusted hook\\n'\n");write_text(sentinel,"preserve-me\n");assert(chmod(src,0644)==0);
 assert(firewall_hook_file_stage(src,dir,".hook-",staged,sizeof(staged))==0);
 assert(stat(staged,&st)==0&&S_ISREG(st.st_mode));assert(st.st_uid==geteuid()&&(st.st_mode&0777)==0600);assert(strcmp(staged,src));
 fp=fopen(staged,"r");assert(fp&&fgets(buf,sizeof(buf),fp));assert(strcmp(buf,"#!/bin/sh\n")==0);assert(fclose(fp)==0);
 {
 char retained[512];
 assert(snprintf(retained,sizeof(retained),"%s",staged)>0);
 fail_unlink_count=1;
 assert(firewall_hook_file_remove_with(staged,test_unlink_once)==-1);
 assert(errno==EACCES&&staged[0]!='\0'&&access(retained,F_OK)==0);
 assert(firewall_hook_file_remove_with(staged,test_unlink_once)==0);
 assert(staged[0]=='\0'&&access(retained,F_OK)!=0&&errno==ENOENT);
}

 assert(chmod(src,0755)==0);assert(firewall_hook_file_stage(src,dir,".hook-",staged,sizeof(staged))==0);
 assert(stat(staged,&st)==0&&(st.st_mode&0777)==0700);assert(firewall_hook_file_remove(staged)==0);assert(chmod(src,0644)==0);
 assert(symlink(src,linkp)==0);assert(firewall_hook_file_stage(linkp,dir,".hook-",staged,sizeof(staged))!=0);
 assert(chmod(src,0660)==0);assert(firewall_hook_file_stage(src,dir,".hook-",staged,sizeof(staged))!=0);assert(chmod(src,0644)==0);
 assert(chmod(src,0602)==0);assert(firewall_hook_file_stage(src,dir,".hook-",staged,sizeof(staged))!=0);assert(chmod(src,0644)==0);
 assert(chmod(src,0666)==0);assert(firewall_hook_file_stage(src,dir,".hook-",staged,sizeof(staged))!=0);assert(chmod(src,0644)==0);
 assert(mkfifo(fifo,0600)==0);assert(firewall_hook_file_stage(fifo,dir,".hook-",staged,sizeof(staged))!=0);assert(unlink(fifo)==0);
 assert(link(src,hard)==0);assert(firewall_hook_file_stage(src,dir,".hook-",staged,sizeof(staged))!=0);assert(unlink(hard)==0);
 assert(chmod(dir,0755)==0);assert(firewall_hook_file_stage(src,dir,".hook-",staged,sizeof(staged))!=0);assert(chmod(dir,0700)==0);
 fd=open(src,O_WRONLY|O_TRUNC);assert(fd>=0&&ftruncate(fd,FIREWALL_HOOK_FILE_MAX_BYTES+1)==0);assert(close(fd)==0);
 assert(firewall_hook_file_stage(src,dir,".hook-",staged,sizeof(staged))!=0&&staged[0]=='\0');
 snprintf(staged,sizeof(staged),"%s/missing",dir);assert(firewall_hook_file_remove(staged)==0&&staged[0]=='\0');
 assert(unlink(linkp)==0);assert(unlink(src)==0);assert(unlink(sentinel)==0);assert(rmdir(dir)==0);assert(rmdir(root)==0);return 0;
}

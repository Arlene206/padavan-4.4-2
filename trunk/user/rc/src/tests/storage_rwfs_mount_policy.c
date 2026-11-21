#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "storage_rwfs_mount_policy.h"
struct mock_ops {
	int module_status, attachment_before, attachment_after;
	int attachment_probe_status, attach_status, prepare_status, mount_status;
	int load_calls, module_probe_calls, attachment_probe_calls;
	int attach_calls, prepare_calls, mount_calls, order[8], order_count;
};
static void record_call(struct mock_ops *m, int call) { assert(m->order_count < 8); m->order[m->order_count++] = call; }
static void load_module(void *p) { struct mock_ops *m=p; m->load_calls++; record_call(m,1); }
static int module_status(void *p) { struct mock_ops *m=p; m->module_probe_calls++; record_call(m,2); return m->module_status; }
static int attachment_status(void *p,int index,int *presence) {
	struct mock_ops *m=p; assert(index==5 && presence); m->attachment_probe_calls++; record_call(m,3);
	if(m->attachment_probe_status) return m->attachment_probe_status;
	*presence=m->attachment_probe_calls==1?m->attachment_before:m->attachment_after; return 0;
}
static int attach_ubi(void *p,int index) { struct mock_ops *m=p; assert(index==5); m->attach_calls++; record_call(m,4); return m->attach_status; }
static int prepare_mountpoint(void *p,const char *path) { struct mock_ops *m=p; assert(strcmp(path,"/media/mtd_rwfs")==0); m->prepare_calls++; record_call(m,5); return m->prepare_status; }
static int mount_ubifs(void *p,int index,const char *path) { struct mock_ops *m=p; assert(index==5 && strcmp(path,"/media/mtd_rwfs")==0); m->mount_calls++; record_call(m,6); return m->mount_status; }
static const rc_rwfs_mount_ops_t ops={load_module,module_status,attachment_status,attach_ubi,prepare_mountpoint,mount_ubifs};
static void reset(struct mock_ops *m) { memset(m,0,sizeof(*m)); m->module_status=1; m->attachment_before=STORAGE_OPTWARE_STOP_RUNNING; m->attachment_after=STORAGE_OPTWARE_STOP_RUNNING; }
static int run(struct mock_ops *m) { return rc_rwfs_mount_execute(1,5,"/media/mtd_rwfs",&ops,m); }
static void test_disabled_and_unsupported(void) { struct mock_ops m; reset(&m); assert(rc_rwfs_mount_execute(0,5,"/media/mtd_rwfs",NULL,NULL)==0); assert(rc_rwfs_mount_execute(1,2,"/media/mtd_rwfs",NULL,NULL)==0); assert(m.order_count==0); }
static void test_invalid(void) { assert(rc_rwfs_mount_execute(1,5,"/media/mtd_rwfs",NULL,NULL)==EINVAL); assert(rc_rwfs_mount_execute(1,5,"",&ops,NULL)==EINVAL); }
static void test_module_states(void) { struct mock_ops m; reset(&m); m.module_status=-EACCES; assert(run(&m)==EACCES&&m.attachment_probe_calls==0); reset(&m); m.module_status=0; assert(run(&m)==ENODEV&&m.attachment_probe_calls==0); reset(&m); m.module_status=2; assert(run(&m)==EIO&&m.attachment_probe_calls==0); }
static void test_unknown_attachment(void) { struct mock_ops m; reset(&m); m.attachment_before=STORAGE_OPTWARE_STOP_UNKNOWN; assert(run(&m)==EIO&&m.attach_calls==0&&m.prepare_calls==0); reset(&m); m.attachment_probe_status=EACCES; assert(run(&m)==EACCES&&m.attach_calls==0); }
static void test_existing_attachment(void) { struct mock_ops m; reset(&m); assert(run(&m)==0); assert(m.attachment_probe_calls==1&&m.attach_calls==0); assert(m.prepare_calls==1&&m.mount_calls==1); assert(m.order_count==5); assert(m.order[0]==1&&m.order[1]==2&&m.order[2]==3&&m.order[3]==5&&m.order[4]==6); }
static void test_attach_requires_postcondition(void) { struct mock_ops m; reset(&m); m.attachment_before=STORAGE_OPTWARE_STOP_ABSENT; m.attachment_after=STORAGE_OPTWARE_STOP_RUNNING; assert(run(&m)==0&&m.attachment_probe_calls==2); assert(m.attach_calls==1&&m.mount_calls==1); assert(m.order[3]==4&&m.order[4]==3&&m.order[5]==5); reset(&m); m.attachment_before=STORAGE_OPTWARE_STOP_ABSENT; m.attachment_after=STORAGE_OPTWARE_STOP_RUNNING; m.attach_status=EIO; assert(run(&m)==0&&m.mount_calls==1); }
static void test_no_mount_on_failed_attach(void) { struct mock_ops m; reset(&m); m.attachment_before=STORAGE_OPTWARE_STOP_ABSENT; m.attachment_after=STORAGE_OPTWARE_STOP_ABSENT; m.attach_status=EIO; assert(run(&m)==EIO&&m.prepare_calls==0&&m.mount_calls==0); reset(&m); m.attachment_before=STORAGE_OPTWARE_STOP_ABSENT; m.attachment_after=STORAGE_OPTWARE_STOP_ABSENT; assert(run(&m)==ENODEV&&m.mount_calls==0); }
static void test_mount_errors(void) { struct mock_ops m; reset(&m); m.prepare_status=EACCES; assert(run(&m)==EACCES&&m.mount_calls==0); reset(&m); m.mount_status=EIO; assert(run(&m)==EIO&&m.mount_calls==1); }
static void test_worker_result_decoder(void)
{
	assert(storage_optware_start_decode_result(-1, 0) ==
	       STORAGE_OPTWARE_START_CHILD_RUNNING);
	assert(storage_optware_start_decode_result(1, 0) ==
	       STORAGE_OPTWARE_START_CHILD_SUCCESS);
	assert(storage_optware_start_decode_result(1, 1) ==
	       STORAGE_OPTWARE_START_CHILD_FAILURE);
	assert(storage_optware_start_decode_result(0, 0) ==
	       STORAGE_OPTWARE_START_CHILD_FAILURE);
}

int main(void) { test_worker_result_decoder(); test_disabled_and_unsupported(); test_invalid(); test_module_states(); test_unknown_attachment(); test_existing_attachment(); test_attach_requires_postcondition(); test_no_mount_on_failed_attach(); test_mount_errors(); puts("RWFS UBI/module/mount fail-closed policy: PASS"); return 0; }

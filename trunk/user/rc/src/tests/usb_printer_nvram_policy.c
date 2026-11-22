#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "usb_printer_nvram_policy.h"
struct fake_nvram { int result; int error; const char *source; char cache[64]; unsigned int calls; char requested_name[32]; };
static int fake_get(void *opaque, const char *name, char **value)
{
    struct fake_nvram *fake=(struct fake_nvram *)opaque;
    fake->calls++;
    snprintf(fake->requested_name,sizeof(fake->requested_name),"%s",name);
    *value=NULL;
    if(fake->result<0){errno=fake->error;return fake->result;}
    if(fake->result==1 && fake->source){snprintf(fake->cache,sizeof(fake->cache),"%s",fake->source);*value=fake->cache;}
    return fake->result;
}
static int copy_setting(struct fake_nvram *fake,char *value,size_t size)
{return usb_printer_nvram_copy_setting_status(fake_get,fake,"rawd_enable",value,size);}
static void test_present_setting_is_copied_from_shared_cache(void)
{
    struct fake_nvram fake={1,0,"2","",0,""}; char value[16];
    assert(copy_setting(&fake,value,sizeof(value))==0 && strcmp(value,"2")==0);
    assert(fake.calls==1 && strcmp(fake.requested_name,"rawd_enable")==0);
}
static void test_missing_setting_preserves_empty_legacy_default(void)
{
    struct fake_nvram fake={0,EACCES,NULL,"",0,""}; char value[16]="stale";
    assert(copy_setting(&fake,value,sizeof(value))==0 && value[0]=='\0');
}
static void test_backend_error_is_not_collapsed_to_default(void)
{
    struct fake_nvram fake={-1,EACCES,NULL,"",0,""}; char value[16]="stale";
    assert(copy_setting(&fake,value,sizeof(value))==EACCES && value[0]=='\0');
}
static void test_errno_less_backend_failure_becomes_eio(void)
{
    struct fake_nvram fake={-1,0,NULL,"",0,""}; char value[16]="stale";
    assert(copy_setting(&fake,value,sizeof(value))==EIO && value[0]=='\0');
}
static void test_unexpected_result_and_null_cache_are_rejected(void)
{
    struct fake_nvram fake={2,0,"2","",0,""}; char value[16]="stale";
    assert(copy_setting(&fake,value,sizeof(value))==EIO && value[0]=='\0');
    fake.result=1; fake.source=NULL;
    assert(copy_setting(&fake,value,sizeof(value))==EIO && value[0]=='\0');
}
static void test_overlong_setting_fails_before_use(void)
{
    struct fake_nvram fake={1,0,"0123456789abcdef","",0,""}; char value[8]="stale";
    assert(copy_setting(&fake,value,sizeof(value))==ENAMETOOLONG && value[0]=='\0');
}
static void test_invalid_arguments_are_rejected(void)
{
    struct fake_nvram fake={0,0,NULL,"",0,""}; char value[16];
    assert(usb_printer_nvram_copy_setting_status(NULL,&fake,"x",value,sizeof(value))==EINVAL);
    assert(usb_printer_nvram_copy_setting_status(fake_get,&fake,NULL,value,sizeof(value))==EINVAL);
    assert(usb_printer_nvram_copy_setting_status(fake_get,&fake,"x",NULL,sizeof(value))==EINVAL);
    assert(usb_printer_nvram_copy_setting_status(fake_get,&fake,"x",value,0)==EINVAL);
    assert(fake.calls==0);
}
int main(void)
{
    test_present_setting_is_copied_from_shared_cache();
    test_missing_setting_preserves_empty_legacy_default();
    test_backend_error_is_not_collapsed_to_default();
    test_errno_less_backend_failure_becomes_eio();
    test_unexpected_result_and_null_cache_are_rejected();
    test_overlong_setting_fails_before_use();
    test_invalid_arguments_are_rejected();
    puts("USB printer checked NVRAM setting regression: PASS");
    return 0;
}

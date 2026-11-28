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
static void test_write_status_accepts_success(void)
{
    assert(usb_printer_nvram_write_status(0, EACCES) == 0);
}
static void test_write_status_preserves_backend_errno(void)
{
    assert(usb_printer_nvram_write_status(-1, ENOSPC) == ENOSPC);
}
static void test_write_status_without_errno_defaults_to_eio(void)
{
    assert(usb_printer_nvram_write_status(-1, 0) == EIO);
}
static void test_write_status_rejects_unexpected_result(void)
{
    assert(usb_printer_nvram_write_status(1, 0) == EIO);
}


static void test_enable_mode_parser_preserves_missing_key_default(void)
{
    int mode = -1;
    assert(usb_printer_enable_mode_status("", 1, &mode) == 0 && mode == 1);
    mode = -1;
    assert(usb_printer_enable_mode_status("", 2, &mode) == 0 && mode == 1);
}
static void test_enable_mode_parser_accepts_supported_modes(void)
{
    int mode = -1;
    assert(usb_printer_enable_mode_status("0", 1, &mode) == 0 && mode == 0);
    assert(usb_printer_enable_mode_status("1", 1, &mode) == 0 && mode == 1);
    assert(usb_printer_enable_mode_status("0", 2, &mode) == 0 && mode == 0);
    assert(usb_printer_enable_mode_status("1", 2, &mode) == 0 && mode == 1);
    assert(usb_printer_enable_mode_status("2", 2, &mode) == 0 && mode == 2);
}
static void test_enable_mode_parser_rejects_unknown_or_multidigit_values(void)
{
    static const char *invalid[] = {"2", "3", "01", "-1", "1\n", "enabled", NULL};
    int index, mode = 77;
    for (index = 0; invalid[index] != NULL; ++index) {
        assert(usb_printer_enable_mode_status(invalid[index], 1, &mode) == EINVAL);
        assert(mode == 77);
        assert(errno == EINVAL);
        mode = 77;
    }
    assert(usb_printer_enable_mode_status("3", 2, &mode) == EINVAL);
    assert(mode == 77);
}
static void test_enable_mode_parser_rejects_invalid_contract_arguments(void)
{
    int mode = 77;
    assert(usb_printer_enable_mode_status(NULL, 1, &mode) == EINVAL);
    assert(usb_printer_enable_mode_status("1", 0, &mode) == EINVAL);
    assert(usb_printer_enable_mode_status("1", 3, &mode) == EINVAL);
    assert(usb_printer_enable_mode_status("1", 1, NULL) == EINVAL);
    assert(mode == 77);
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
    test_write_status_accepts_success();
    test_write_status_preserves_backend_errno();
    test_write_status_without_errno_defaults_to_eio();
    test_write_status_rejects_unexpected_result();
    test_enable_mode_parser_preserves_missing_key_default();
    test_enable_mode_parser_accepts_supported_modes();
    test_enable_mode_parser_rejects_unknown_or_multidigit_values();
    test_enable_mode_parser_rejects_invalid_contract_arguments();
    puts("USB printer checked NVRAM setting regression: PASS");
    return 0;
}

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "upnp_restart_policy.h"
#include "rc_notify_batch_policy.h"
static int stop_calls;
static int firewall_calls;
static int stop_status;
static int mock_stop(void) { stop_calls++; return stop_status; }
static void mock_firewall_restart(void) { firewall_calls++; }
static void reset_case(int status) { stop_calls=0; firewall_calls=0; stop_status=status; }
static void test_success_stops_then_restarts_firewall(void) { reset_case(0); assert(rc_upnp_restart_run(mock_stop,mock_firewall_restart)==0); assert(stop_calls==1&&firewall_calls==1); assert(rc_notify_merge_action_result(0,0)==0); }
static void test_stop_failure_aborts_firewall_restart_and_propagates(void) { reset_case(EBUSY); assert(rc_upnp_restart_run(mock_stop,mock_firewall_restart)==EBUSY); assert(stop_calls==1&&firewall_calls==0); assert(rc_notify_merge_action_result(0,stop_status)==-1); }
static void test_stop_error_codes_are_preserved(void) { reset_case(EIO); assert(rc_upnp_restart_run(mock_stop,mock_firewall_restart)==EIO); assert(firewall_calls==0); }
static void test_invalid_callbacks_fail_without_side_effects(void) { reset_case(0); assert(rc_upnp_restart_run(NULL,mock_firewall_restart)==EINVAL); assert(rc_upnp_restart_run(mock_stop,NULL)==EINVAL); assert(stop_calls==0&&firewall_calls==0); }
int main(void) { test_success_stops_then_restarts_firewall(); test_stop_failure_aborts_firewall_restart_and_propagates(); test_stop_error_codes_are_preserved(); test_invalid_callbacks_fail_without_side_effects(); puts("RC UPnP restart stop-result policy: PASS"); return 0; }

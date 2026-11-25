#include <assert.h>
#include <stdio.h>
#include <sys/types.h>

#include "vpn_client_callback_policy.h"

int
main(void)
{
	assert(vpn_client_callback_interface_is_valid("ppp0"));
	assert(vpn_client_callback_interface_is_valid("ppp10"));
	assert(vpn_client_callback_interface_is_valid("ppp255"));
	assert(!vpn_client_callback_interface_is_valid(NULL));
	assert(!vpn_client_callback_interface_is_valid(""));
	assert(!vpn_client_callback_interface_is_valid("eth0"));
	assert(!vpn_client_callback_interface_is_valid("ppp"));
	assert(!vpn_client_callback_interface_is_valid("ppp00"));
	assert(!vpn_client_callback_interface_is_valid("ppp010"));
	assert(!vpn_client_callback_interface_is_valid("ppp10x"));
	assert(!vpn_client_callback_interface_is_valid("ppp256"));

	assert(vpn_client_callback_session_matches(
		"ppp10", "12345", (pid_t)12345, (pid_t)12345));
	assert(!vpn_client_callback_session_matches(
		"ppp10", "12345", (pid_t)12346, (pid_t)12345));
	assert(!vpn_client_callback_session_matches(
		"ppp10", "12345", (pid_t)12345, (pid_t)12346));
	assert(!vpn_client_callback_session_matches(
		"ppp10", "12345", (pid_t)1, (pid_t)1));
	assert(!vpn_client_callback_session_matches(
		"ppp256", "12345", (pid_t)12345, (pid_t)12345));
	assert(!vpn_client_callback_session_matches(
		"ppp10", "12x", (pid_t)12345, (pid_t)12345));
	assert(!vpn_client_callback_session_matches(
		"ppp10", NULL, (pid_t)12345, (pid_t)12345));

	puts("VPN client PPP callback identity policy: PASS");
	return 0;
}

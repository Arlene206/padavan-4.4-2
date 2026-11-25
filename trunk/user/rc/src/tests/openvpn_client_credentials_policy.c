#include <assert.h>
#include <string.h>

#include "openvpn_client_credentials_policy.h"

struct credential_fixture {
	char username[16];
	char password[24];
};

static void
test_credentials_are_cleared(void)
{
	struct credential_fixture credentials;
	static const unsigned char zero_username[sizeof(credentials.username)] = { 0 };
	static const unsigned char zero_password[sizeof(credentials.password)] = { 0 };

	memset(&credentials, 0, sizeof(credentials));
	strcpy(credentials.username, "router-user");
	strcpy(credentials.password, "secret-token");

	openvpn_client_credentials_secure_clear(credentials.username,
						 sizeof(credentials.username));
	openvpn_client_credentials_secure_clear(credentials.password,
						 sizeof(credentials.password));

	assert(memcmp(credentials.username, zero_username,
		      sizeof(credentials.username)) == 0);
	assert(memcmp(credentials.password, zero_password,
		      sizeof(credentials.password)) == 0);
}

static void
test_null_clear_is_safe(void)
{
	openvpn_client_credentials_secure_clear(NULL, 32);
}

int
main(void)
{
	test_credentials_are_cleared();
	test_null_clear_is_safe();
	return 0;
}

#ifndef UQMI_PRIVATE_CREDENTIALS_H
#define UQMI_PRIVATE_CREDENTIALS_H

#define UQMI_PRIVATE_PIN_SIZE 64
#define UQMI_PRIVATE_VALUE_SIZE 256
#define UQMI_PRIVATE_CREDENTIALS_MAX 1024

struct uqmi_private_credentials {
	char pin[UQMI_PRIVATE_PIN_SIZE + 1];
	char username[UQMI_PRIVATE_VALUE_SIZE + 1];
	char password[UQMI_PRIVATE_VALUE_SIZE + 1];
	int has_pin;
	int has_username;
	int has_password;
};

/*
 * Load one private, one-shot credential file. A valid file is removed from
 * its pinned parent directory before its contents are parsed or returned.
 */
int uqmi_private_credentials_load(const char *path,
				  struct uqmi_private_credentials *credentials);
void uqmi_private_credentials_clear(struct uqmi_private_credentials *credentials);

#endif

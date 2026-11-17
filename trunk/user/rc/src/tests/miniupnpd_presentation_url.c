#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "miniupnpd_presentation_url.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int test_default_ports_omit_suffix(void)
{
	char url[32];
	CHECK(miniupnpd_format_lan_url("192.168.1.1", 0, url, sizeof(url)) == 0);
	CHECK(strcmp(url, "192.168.1.1") == 0);
	CHECK(miniupnpd_format_lan_url("192.168.1.1", 80, url, sizeof(url)) == 0);
	CHECK(strcmp(url, "192.168.1.1") == 0);
	return 0;
}

static int test_custom_port_is_appended(void)
{
	char url[32];
	CHECK(miniupnpd_format_lan_url("192.168.1.1", 8080, url, sizeof(url)) == 0);
	CHECK(strcmp(url, "192.168.1.1:8080") == 0);
	CHECK(miniupnpd_format_lan_url("10.0.0.254", 65535, url, sizeof(url)) == 0);
	CHECK(strcmp(url, "10.0.0.254:65535") == 0);
	return 0;
}

static int test_oversized_address_fails_without_truncation(void)
{
	char address[256], url[32];
	memset(address, '9', sizeof(address));
	address[sizeof(address) - 1] = '\0';
	strcpy(url, "stale");
	errno = 0;
	CHECK(miniupnpd_format_lan_url(address, 8080, url, sizeof(url)) == -1);
	CHECK(errno == ENOSPC);
	CHECK(url[0] == '\0');
	return 0;
}

static int test_invalid_port_and_null_address_fail(void)
{
	char url[32];
	strcpy(url, "stale");
	errno = 0;
	CHECK(miniupnpd_format_lan_url("192.168.1.1", -1, url, sizeof(url)) == -1);
	CHECK(errno == EINVAL && url[0] == '\0');
	errno = 0;
	CHECK(miniupnpd_format_lan_url("192.168.1.1", 65536, url, sizeof(url)) == -1);
	CHECK(errno == EINVAL && url[0] == '\0');
	errno = 0;
	CHECK(miniupnpd_format_lan_url(NULL, 80, url, sizeof(url)) == -1);
	CHECK(errno == EINVAL && url[0] == '\0');
	return 0;
}

static int test_invalid_output_buffer_fails(void)
{
	char url[8];
	errno = 0;
	CHECK(miniupnpd_format_lan_url("192.168.1.1", 80, NULL, sizeof(url)) == -1);
	CHECK(errno == EINVAL);
	strcpy(url, "stale");
	errno = 0;
	CHECK(miniupnpd_format_lan_url("192.168.1.1", 80, url, 0) == -1);
	CHECK(errno == EINVAL && strcmp(url, "stale") == 0);
	return 0;
}

static int test_short_output_buffer_is_cleared(void)
{
	char url[10];
	strcpy(url, "stale");
	errno = 0;
	CHECK(miniupnpd_format_lan_url("192.168.1.1", 80, url, sizeof(url)) == -1);
	CHECK(errno == ENOSPC && url[0] == '\0');
	return 0;
}

int main(void)
{
	CHECK(test_default_ports_omit_suffix() == 0);
	CHECK(test_custom_port_is_appended() == 0);
	CHECK(test_oversized_address_fails_without_truncation() == 0);
	CHECK(test_invalid_port_and_null_address_fail() == 0);
	CHECK(test_invalid_output_buffer_fails() == 0);
	CHECK(test_short_output_buffer_is_cleared() == 0);
	puts("MiniUPnP presentation URL bounds regression: PASS");
	return 0;
}

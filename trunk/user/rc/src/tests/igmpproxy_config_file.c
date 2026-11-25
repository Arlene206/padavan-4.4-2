#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>

#include "igmpproxy_config_file.h"

static void
write_file(const char *path, const char *contents)
{
	FILE *stream = fopen(path, "w");
	size_t length = strlen(contents);

	assert(stream != NULL);
	assert(fwrite(contents, 1, length, stream) == length);
	assert(fclose(stream) == 0);
}

static void
assert_file_equals(const char *path, const char *expected)
{
	char buffer[2048];
	FILE *stream;
	size_t length;

	stream = fopen(path, "r");
	assert(stream != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, stream);
	assert(!ferror(stream));
	assert(feof(stream));
	buffer[length] = '\0';
	assert(fclose(stream) == 0);
	assert(strcmp(buffer, expected) == 0);
}

static void
assert_no_temporary_files(const char *directory, const char *prefix)
{
	DIR *stream;
	struct dirent *entry;
	size_t prefix_length = strlen(prefix);

	stream = opendir(directory);
	assert(stream != NULL);
	while ((entry = readdir(stream)) != NULL)
		assert(strncmp(entry->d_name, prefix, prefix_length) != 0);
	assert(closedir(stream) == 0);
}

int
main(void)
{
	char directory_template[] = "/tmp/igmpproxy-config.XXXXXX";
	char *directory;
	char config_path[512];
	char missing_path[512];
	char directory_path[512];
	struct igmpproxy_config_values values;
	struct stat st;
	struct rlimit saved_limit;
	struct rlimit constrained_limit;
	void (*old_sigxfsz)(int);
	int status;
	unsigned int i;
	static const char *const invalid_altnets[] = {
		"bad", "192.0.2.256/24", "256.0.2.1/24",
		"192.0.2.1/33", "192.0.2/24", "192.0.2.1",
		"192.0.2.1/24/8", "192.0.2.1/",
		"010.0.0.0/8", "192.168.001.1/24",
		"255.255.255.255/32"
	};

	directory = mkdtemp(directory_template);
	assert(directory != NULL);
	assert(snprintf(config_path, sizeof(config_path), "%s/igmpproxy.conf",
			directory) > 0);
	assert(snprintf(missing_path, sizeof(missing_path),
			"%s/missing/igmpproxy.conf", directory) > 0);
	assert(snprintf(directory_path, sizeof(directory_path),
			"%s/target.conf", directory) > 0);

	values.upstream_interface = "eth0.2";
	values.altnet = "192.0.2.0/24";
	values.downstream_interface = "br0";
	values.quickleave = 1;
	status = igmpproxy_config_publish(config_path, &values);
	assert(status == 0);
	assert_file_equals(config_path,
		"# automatically generated\n"
		"quickleave\n"
		"\n"
		"phyint eth0.2 upstream ratelimit 0  threshold 1\n"
		"\taltnet 192.0.2.0/24\n"
		"\nphyint br0 downstream ratelimit 0  threshold 1\n"
		"\n");
	assert(stat(config_path, &st) == 0);
	assert((st.st_mode & 0777) == 0600);
	assert_no_temporary_files(directory, "igmpproxy.conf.tmp.");

	/* Preserve legacy default altnet and disabled quickleave output. */
	values.altnet = (char *)igmpproxy_config_altnet_or_default("");
	values.quickleave = 0;
	status = igmpproxy_config_publish(config_path, &values);
	assert(status == 0);
	assert_file_equals(config_path,
		"# automatically generated\n"
		"\n"
		"phyint eth0.2 upstream ratelimit 0  threshold 1\n"
		"\taltnet 0.0.0.0/0\n"
		"\nphyint br0 downstream ratelimit 0  threshold 1\n"
		"\n");

	/* Empty settings keep the legacy default, but malformed short values
	 * must not silently widen the accepted multicast source network.
	 */
	assert(strcmp(igmpproxy_config_altnet_or_default(""),
		      "0.0.0.0/0") == 0);
	assert(strcmp(igmpproxy_config_altnet_or_default("bad"), "bad") == 0);
	assert(igmpproxy_config_altnet_token("0.0.0.0/0"));
	assert(igmpproxy_config_altnet_token("255.255.255.254/32"));
	assert(igmpproxy_config_altnet_token("192.168.1.1/24"));
	write_file(config_path, "last-good-altnet-validation\n");
	for (i = 0; i < sizeof(invalid_altnets) / sizeof(invalid_altnets[0]); ++i) {
		values.altnet = (char *)invalid_altnets[i];
		assert(!igmpproxy_config_altnet_token(values.altnet));
		assert(igmpproxy_config_publish(config_path, &values) == EINVAL);
		assert_file_equals(config_path, "last-good-altnet-validation\n");
	}
	values.altnet = "192.0.2.0/24";
	values.quickleave = 1;

	/* Reject line injection and leave the live configuration untouched. */
	write_file(config_path, "last-good-configuration\n");
	values.altnet = "192.0.2.0/24\nquickleave";
	assert(igmpproxy_config_publish(config_path, &values) == EINVAL);
	assert_file_equals(config_path, "last-good-configuration\n");
	values.altnet = "192.0.2.0/24";
	values.quickleave = 1;

	/* Reject whitespace/comment token injection even without a newline. */
	write_file(config_path, "last-good-token-validation\n");
	values.altnet = "192.0.2.0/24 quickleave";
	assert(igmpproxy_config_publish(config_path, &values) == EINVAL);
	assert_file_equals(config_path, "last-good-token-validation\n");
	values.altnet = "192.0.2.0/24";

	values.upstream_interface = "eth0.2 #comment";
	assert(igmpproxy_config_publish(config_path, &values) == EINVAL);
	assert_file_equals(config_path, "last-good-token-validation\n");
	values.upstream_interface = "eth0.2";

	values.downstream_interface = "br0\tphyint";
	assert(igmpproxy_config_publish(config_path, &values) == EINVAL);
	assert_file_equals(config_path, "last-good-token-validation\n");
	values.downstream_interface = "br0";
	assert_no_temporary_files(directory, "igmpproxy.conf.tmp.");
	write_file(config_path, "last-good-configuration\n");

	/* Missing-parent and rename failures never replace the previous config. */
	status = igmpproxy_config_publish(missing_path, &values);
	assert(status != 0);
	assert_file_equals(config_path, "last-good-configuration\n");
	assert_no_temporary_files(directory, "igmpproxy.conf.tmp.");

	assert(mkdir(directory_path, 0700) == 0);
	status = igmpproxy_config_publish(directory_path, &values);
	assert(status != 0);
	assert(stat(directory_path, &st) == 0 && S_ISDIR(st.st_mode));
	assert_no_temporary_files(directory, "target.conf.tmp.");

	/* A small RLIMIT_FSIZE injects a buffered write/flush failure. */
	write_file(config_path, "last-good-after-write-failure\n");
	assert(getrlimit(RLIMIT_FSIZE, &saved_limit) == 0);
	assert(saved_limit.rlim_max == RLIM_INFINITY || saved_limit.rlim_max >= 1);
	constrained_limit = saved_limit;
	constrained_limit.rlim_cur = 1;
	old_sigxfsz = signal(SIGXFSZ, SIG_IGN);
	assert(old_sigxfsz != SIG_ERR);
	assert(setrlimit(RLIMIT_FSIZE, &constrained_limit) == 0);
	status = igmpproxy_config_publish(config_path, &values);
	assert(setrlimit(RLIMIT_FSIZE, &saved_limit) == 0);
	assert(signal(SIGXFSZ, old_sigxfsz) != SIG_ERR);
	assert(status != 0);
	assert_file_equals(config_path, "last-good-after-write-failure\n");
	assert_no_temporary_files(directory, "igmpproxy.conf.tmp.");

	assert(unlink(config_path) == 0);
	assert(rmdir(directory_path) == 0);
	assert(rmdir(directory) == 0);
	puts("IGMP proxy atomic configuration publication: PASS");
	return 0;
}

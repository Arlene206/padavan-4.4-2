#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dnsmasq_file.h"

static int discard_mock_result;
static int discard_mock_error;
static int discard_mock_calls;
static int discard_mock_eintr_count;
static const char *discard_mock_expected_path = "/tmp/dnsmasq-discard-test.stage";

static int
mock_discard_unlink(const char *path)
{
	assert(strcmp(path, discard_mock_expected_path) == 0);
	discard_mock_calls++;
	if (discard_mock_eintr_count > 0) {
		discard_mock_eintr_count--;
		errno = EINTR;
		return -1;
	}
	if (discard_mock_result != 0) {
		errno = discard_mock_error;
		return -1;
	}
	return 0;
}

static void
test_discard_preserves_path_when_unlink_fails(void)
{
	char stage[] = "/tmp/dnsmasq-discard-test.stage";

	discard_mock_result = -1;
	discard_mock_error = EACCES;
	discard_mock_calls = 0;
	errno = 0;
	assert(dnsmasq_file_discard_with_unlink(stage, mock_discard_unlink) == -1);
	assert(errno == EACCES);
	assert(strcmp(stage, "/tmp/dnsmasq-discard-test.stage") == 0);
	assert(discard_mock_calls == 1);

	/* A caller can retry because the failed unlink did not erase the path. */
	discard_mock_result = 0;
	assert(dnsmasq_file_discard_with_unlink(stage, mock_discard_unlink) == 0);
	assert(stage[0] == '\0');
	assert(discard_mock_calls == 2);

	/* ENOENT means the requested cleanup state already holds. */
	strcpy(stage, "/tmp/dnsmasq-discard-test.stage");
	discard_mock_result = -1;
	discard_mock_error = ENOENT;
	assert(dnsmasq_file_discard_with_unlink(stage, mock_discard_unlink) == 0);
	assert(stage[0] == '\0');
	assert(discard_mock_calls == 3);

	/* Rollback snapshot cleanup exposes the same retryable error contract. */
	strcpy(stage, "/tmp/dnsmasq-discard-test.stage");
	discard_mock_result = -1;
	discard_mock_error = EACCES;
	assert(dnsmasq_file_backup_discard_with_unlink(stage, mock_discard_unlink) == -1);
	assert(errno == EACCES);
	assert(strcmp(stage, "/tmp/dnsmasq-discard-test.stage") == 0);
	discard_mock_result = 0;
	assert(dnsmasq_file_backup_discard_with_unlink(stage, mock_discard_unlink) == 0);
	assert(stage[0] == '\0');

	strcpy(stage, "/tmp/dnsmasq-discard-test.stage");
	assert(dnsmasq_file_discard_with_unlink(stage, NULL) == -1);
	assert(errno == EINVAL);
	assert(strcmp(stage, "/tmp/dnsmasq-discard-test.stage") == 0);
}

static FILE *
mock_fdopen_failure(int fd, const char *mode)
{
	(void)fd;
	(void)mode;
	errno = ENOMEM;
	return NULL;
}

static void
test_open_failure_retains_path_for_retry(const char *directory)
{
	char stage[256];

	assert(snprintf(stage, sizeof(stage), "%s/fdopen-fail.XXXXXX", directory) > 0);
	discard_mock_expected_path = stage;
	discard_mock_result = -1;
	discard_mock_error = EACCES;
	discard_mock_eintr_count = 0;
	discard_mock_calls = 0;
	errno = 0;
	assert(dnsmasq_file_open_with_ops(stage, mock_fdopen_failure, close,
					 mock_discard_unlink) == NULL);
	assert(errno == ENOMEM);
	assert(stage[0] != '\0');
	assert(access(stage, F_OK) == 0);
	assert(discard_mock_calls == 1);

	/* A later caller retry has the original unique pathname. */
	assert(dnsmasq_file_discard(stage) == 0);
	assert(stage[0] == '\0');
	assert(access(stage, F_OK) != 0);
	discard_mock_expected_path = "/tmp/dnsmasq-discard-test.stage";
}

static void
test_discard_retries_interrupted_unlink(void)
{
	char stage[] = "/tmp/dnsmasq-discard-test.stage";

	discard_mock_result = 0;
	discard_mock_eintr_count = 1;
	discard_mock_calls = 0;
	assert(dnsmasq_file_discard_with_unlink(stage, mock_discard_unlink) == 0);
	assert(stage[0] == '\0');
	assert(discard_mock_calls == 2);
}

static void
read_exact(const char *path, const char *expected)
{
	char buf[128];
	FILE *fp = fopen(path, "r");
	assert(fp != NULL);
	assert(fgets(buf, sizeof(buf), fp) != NULL);
	assert(strcmp(buf, expected) == 0);
	assert(fclose(fp) == 0);
}


static void
replace_file_contents(const char *destination, const char *contents)
{
	char stage[320];
	FILE *fp;

	assert(snprintf(stage, sizeof(stage), "%s.stage.XXXXXX", destination) > 0);
	fp = dnsmasq_file_open(stage);
	assert(fp != NULL);
	assert(fputs(contents, fp) >= 0);
	assert(dnsmasq_file_publish(fp, stage, destination, 0644) == 0);
}

static void
test_snapshot_set_restore_and_commit(const char *directory)
{
	char first[256], second[256], third[256];
	char first_backup[320], second_backup[320], third_backup[320];
	char *backups[3];
	const char *destinations[3];
	FILE *fp;

	assert(snprintf(first, sizeof(first), "%s/snapshot-first", directory) > 0);
	assert(snprintf(second, sizeof(second), "%s/snapshot-second", directory) > 0);
	assert(snprintf(third, sizeof(third), "%s/snapshot-third", directory) > 0);
	assert(snprintf(first_backup, sizeof(first_backup),
			"%s/snapshot-first.rollback.XXXXXX", directory) > 0);
	assert(snprintf(second_backup, sizeof(second_backup),
			"%s/snapshot-second.rollback.XXXXXX", directory) > 0);
	assert(snprintf(third_backup, sizeof(third_backup),
			"%s/snapshot-third.rollback.XXXXXX", directory) > 0);
	destinations[0] = first;
	destinations[1] = second;
	destinations[2] = third;
	backups[0] = first_backup;
	backups[1] = second_backup;
	backups[2] = third_backup;

	fp = fopen(first, "w");
	assert(fp != NULL && fputs("old-first\n", fp) >= 0 && fclose(fp) == 0);
	fp = fopen(third, "w");
	assert(fp != NULL && fputs("old-third\n", fp) >= 0 && fclose(fp) == 0);

	/* Rollback restores both old files and removes a formerly absent output. */
	assert(dnsmasq_file_snapshot_set_create(destinations, backups, 3) == 0);
	assert(first_backup[0] != '\0' && second_backup[0] == '\0' &&
	       third_backup[0] != '\0');
	replace_file_contents(first, "new-first\n");
	replace_file_contents(second, "new-second\n");
	replace_file_contents(third, "new-third\n");
	assert(dnsmasq_file_snapshot_set_restore(destinations, backups, 3) == 0);
	read_exact(first, "old-first\n");
	read_exact(third, "old-third\n");
	assert(access(second, F_OK) != 0 && errno == ENOENT);
	assert(first_backup[0] == '\0' && second_backup[0] == '\0' &&
	       third_backup[0] == '\0');

	/* Commit discards rollback links but retains every replacement file. */
	assert(dnsmasq_file_snapshot_set_create(destinations, backups, 3) == 0);
	replace_file_contents(first, "committed-first\n");
	replace_file_contents(second, "committed-second\n");
	replace_file_contents(third, "committed-third\n");
	assert(dnsmasq_file_snapshot_set_discard(destinations, backups, 3) == 0);
	assert(first_backup[0] == '\0' && second_backup[0] == '\0' &&
	       third_backup[0] == '\0');
	read_exact(first, "committed-first\n");
	read_exact(second, "committed-second\n");
	read_exact(third, "committed-third\n");

	assert(unlink(first) == 0 && unlink(second) == 0 && unlink(third) == 0);
}

static int
test_xupnpd_pair_preserves_modes_and_rolls_back(void)
{
	char directory[] = "/tmp/dnsmasq-pair-XXXXXX";
	char first[160], second[160];
	char first_stage[192], second_stage[192];
	char first_backup[224], second_backup[224];
	FILE *first_fp, *second_fp;
	struct stat st;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(first, sizeof(first), "%s/common.lua", directory) > 0);
	assert(snprintf(second, sizeof(second), "%s/xupnpd.lua", directory) > 0);

	/* A successful pair commit preserves both destination modes. */
	first_fp = fopen(first, "w");
	assert(first_fp != NULL && fputs("old-common\n", first_fp) >= 0);
	assert(fclose(first_fp) == 0);
	second_fp = fopen(second, "w");
	assert(second_fp != NULL && fputs("old-runtime\n", second_fp) >= 0);
	assert(fclose(second_fp) == 0);
	assert(chmod(first, 0600) == 0 && chmod(second, 0640) == 0);

	assert(snprintf(first_stage, sizeof(first_stage), "%s/common.stage.XXXXXX", directory) > 0);
	assert(snprintf(second_stage, sizeof(second_stage), "%s/runtime.stage.XXXXXX", directory) > 0);
	assert(snprintf(first_backup, sizeof(first_backup), "%s/common.rollback.XXXXXX", directory) > 0);
	assert(snprintf(second_backup, sizeof(second_backup), "%s/runtime.rollback.XXXXXX", directory) > 0);
	first_fp = dnsmasq_file_open(first_stage);
	second_fp = dnsmasq_file_open(second_stage);
	assert(first_fp != NULL && second_fp != NULL);
	assert(fchmod(fileno(first_fp), 0600) == 0);
	assert(fchmod(fileno(second_fp), 0640) == 0);
	assert(fputs("new-common\n", first_fp) >= 0);
	assert(fputs("new-runtime\n", second_fp) >= 0);
	assert(dnsmasq_file_publish_pair(first_fp, first_stage, first, first_backup,
					 second_fp, second_stage, second, second_backup,
					 DNSMASQ_FILE_PRESERVE_MODE) == 0);
	read_exact(first, "new-common\n");
	read_exact(second, "new-runtime\n");
	assert(stat(first, &st) == 0 && (st.st_mode & 0777) == 0600);
	assert(stat(second, &st) == 0 && (st.st_mode & 0777) == 0640);
	assert(first_stage[0] == '\0' && second_stage[0] == '\0');
	assert(first_backup[0] == '\0' && second_backup[0] == '\0');

	/* A real second rename failure must restore both old files and modes. */
	first_fp = fopen(first, "w");
	assert(first_fp != NULL && fputs("rollback-common\n", first_fp) >= 0);
	assert(fclose(first_fp) == 0);
	second_fp = fopen(second, "w");
	assert(second_fp != NULL && fputs("rollback-runtime\n", second_fp) >= 0);
	assert(fclose(second_fp) == 0);
	assert(chmod(first, 0600) == 0 && chmod(second, 0640) == 0);

	assert(snprintf(first_stage, sizeof(first_stage), "%s/common.rollback-stage.XXXXXX", directory) > 0);
	assert(snprintf(second_stage, sizeof(second_stage), "%s/runtime.rollback-stage.XXXXXX", directory) > 0);
	assert(snprintf(first_backup, sizeof(first_backup), "%s/common.rollback.XXXXXX", directory) > 0);
	assert(snprintf(second_backup, sizeof(second_backup), "%s/runtime.rollback.XXXXXX", directory) > 0);
	first_fp = dnsmasq_file_open(first_stage);
	second_fp = dnsmasq_file_open(second_stage);
	assert(first_fp != NULL && second_fp != NULL);
	assert(fchmod(fileno(first_fp), 0600) == 0);
	assert(fchmod(fileno(second_fp), 0640) == 0);
	assert(fputs("not-committed-common\n", first_fp) >= 0);
	assert(fputs("not-committed-runtime\n", second_fp) >= 0);
	assert(unlink(second_stage) == 0);
	errno = 0;
	assert(dnsmasq_file_publish_pair(first_fp, first_stage, first, first_backup,
					 second_fp, second_stage, second, second_backup,
					 DNSMASQ_FILE_PRESERVE_MODE) == -1);
	assert(errno == ENOENT);
	read_exact(first, "rollback-common\n");
	read_exact(second, "rollback-runtime\n");
	assert(stat(first, &st) == 0 && (st.st_mode & 0777) == 0600);
	assert(stat(second, &st) == 0 && (st.st_mode & 0777) == 0640);
	assert(first_stage[0] == '\0' && second_stage[0] == '\0');
	assert(first_backup[0] == '\0' && second_backup[0] == '\0');

	assert(unlink(first) == 0 && unlink(second) == 0);
	assert(rmdir(directory) == 0);
	return 0;
}

int
main(void)
{
	assert(test_xupnpd_pair_preserves_modes_and_rolls_back() == 0);
	char directory[] = "/tmp/dnsmasq-file-test.XXXXXX";
	char sentinel[256], destination[256], template[256], staged[256];
	char fail_template[256], fail_path[256], rename_fail_template[256], rename_fail_path[256], lease_path[256];
	char config_destination[256], config_template[256];
	char config_fail_template[256], config_fail_path[256];
	char resolv_destination[256], resolv_same_template[256], resolv_diff_template[256];
	char resolv_symlink[256], resolv_link_template[256];
	char symlink_lease[256], hardlink_lease[256], fifo_lease[256], private_lease[256];
	char unsafe_read[256];
	char pair_first[256], pair_second[256], pair_bad_second[256], pair_sentinel[256];
	char pair_first_stage[256], pair_second_stage[256];
	char pair_first_backup[256], pair_second_backup[256];
	char pair_absent_first[256], pair_absent_stage[256], pair_absent_backup[256];
	char set_first[256], set_second[256], set_third[256], set_bad_third[256];
	char set_stage_one[256], set_stage_two[256], set_stage_three[256];
	char set_backup_one[256], set_backup_two[256], set_backup_three[256];
	char *set_stages[3], *set_backups[3];
	const char *set_destinations[3];
	FILE *set_files[3];
	struct stat pair_stat;
	FILE *fp, *pair_fp;
	struct stat st;
	int saved_errno;

	test_discard_preserves_path_when_unlink_fails();

	assert(dnsmasq_valid_server_domain("example.com"));
	assert(dnsmasq_valid_server_domain("sub-domain.example.co"));
	assert(dnsmasq_valid_server_domain("router"));
	assert(dnsmasq_valid_server_domain("example.com."));
	assert(!dnsmasq_valid_server_domain(NULL));
	assert(!dnsmasq_valid_server_domain(""));
	assert(!dnsmasq_valid_server_domain(".example.com"));
	assert(!dnsmasq_valid_server_domain("example..com"));
	assert(!dnsmasq_valid_server_domain("-bad.example"));
	assert(!dnsmasq_valid_server_domain("bad-.example"));
	assert(!dnsmasq_valid_server_domain("bad domain"));
	assert(!dnsmasq_valid_server_domain("example.com/8.8.8.8"));
	assert(!dnsmasq_valid_server_domain("evil.example\nserver=//203.0.113.9#"));
	assert(!dnsmasq_valid_server_domain("evil.example\raddress=/blocked.test/127.0.0.1"));

	/* DNS-only and RA-only modes do not use the lease database. */
	assert(!dnsmasq_lease_file_required(0));
	assert(!dnsmasq_lease_file_required(DNSMASQ_DHCP_FEATURE_RA));
	assert(dnsmasq_lease_file_required(DNSMASQ_DHCP_FEATURE_LEASES));
	assert(dnsmasq_lease_file_required(DNSMASQ_DHCP_FEATURE_LEASES |
					 DNSMASQ_DHCP_FEATURE_RA));

	assert(mkdtemp(directory) != NULL);
	test_snapshot_set_restore_and_commit(directory);
	test_open_failure_retains_path_for_retry(directory);
	test_discard_retries_interrupted_unlink();
	assert(snprintf(sentinel, sizeof(sentinel), "%s/sentinel", directory) > 0);
	assert(snprintf(destination, sizeof(destination), "%s/dnsmasq.servers", directory) > 0);
	assert(snprintf(template, sizeof(template), "%s/dnsmasq.servers.XXXXXX", directory) > 0);
	assert(snprintf(fail_template, sizeof(fail_template), "%s/failed.XXXXXX", directory) > 0);
	assert(snprintf(rename_fail_template, sizeof(rename_fail_template), "%s/rename-fail.XXXXXX", directory) > 0);
	assert(snprintf(lease_path, sizeof(lease_path), "%s/dnsmasq.leases", directory) > 0);
	assert(snprintf(symlink_lease, sizeof(symlink_lease), "%s/symlink.leases", directory) > 0);
	assert(snprintf(hardlink_lease, sizeof(hardlink_lease), "%s/hardlink.leases", directory) > 0);
	assert(snprintf(fifo_lease, sizeof(fifo_lease), "%s/fifo.leases", directory) > 0);
	assert(snprintf(private_lease, sizeof(private_lease), "%s/private.leases", directory) > 0);
	assert(snprintf(unsafe_read, sizeof(unsafe_read), "%s/unsafe-readable", directory) > 0);
	assert(snprintf(pair_first, sizeof(pair_first), "%s/pair.hostname", directory) > 0);
	assert(snprintf(pair_second, sizeof(pair_second), "%s/pair.hosts", directory) > 0);
	assert(snprintf(pair_bad_second, sizeof(pair_bad_second), "%s/missing/hosts", directory) > 0);
	assert(snprintf(pair_sentinel, sizeof(pair_sentinel), "%s/pair-sentinel", directory) > 0);
	assert(snprintf(pair_first_stage, sizeof(pair_first_stage), "%s/pair.hostname.XXXXXX", directory) > 0);
	assert(snprintf(pair_second_stage, sizeof(pair_second_stage), "%s/pair.hosts.XXXXXX", directory) > 0);
	assert(snprintf(pair_first_backup, sizeof(pair_first_backup), "%s/pair.hostname.rollback.XXXXXX", directory) > 0);
	assert(snprintf(pair_second_backup, sizeof(pair_second_backup), "%s/pair.hosts.rollback.XXXXXX", directory) > 0);
	assert(snprintf(pair_absent_first, sizeof(pair_absent_first), "%s/pair.absent", directory) > 0);
	assert(snprintf(pair_absent_stage, sizeof(pair_absent_stage), "%s/pair.absent.XXXXXX", directory) > 0);
	assert(snprintf(pair_absent_backup, sizeof(pair_absent_backup), "%s/pair.absent.rollback.XXXXXX", directory) > 0);
	assert(snprintf(set_first, sizeof(set_first), "%s/set-one", directory) > 0);
	assert(snprintf(set_second, sizeof(set_second), "%s/set-two", directory) > 0);
	assert(snprintf(set_third, sizeof(set_third), "%s/set-three", directory) > 0);
	assert(snprintf(set_bad_third, sizeof(set_bad_third), "%s/missing/set-three", directory) > 0);
	assert(snprintf(set_stage_one, sizeof(set_stage_one), "%s/set-one.XXXXXX", directory) > 0);
	assert(snprintf(set_stage_two, sizeof(set_stage_two), "%s/set-two.XXXXXX", directory) > 0);
	assert(snprintf(set_stage_three, sizeof(set_stage_three), "%s/set-three.XXXXXX", directory) > 0);
	assert(snprintf(set_backup_one, sizeof(set_backup_one), "%s/set-one.rollback.XXXXXX", directory) > 0);
	assert(snprintf(set_backup_two, sizeof(set_backup_two), "%s/set-two.rollback.XXXXXX", directory) > 0);
	assert(snprintf(set_backup_three, sizeof(set_backup_three), "%s/set-three.rollback.XXXXXX", directory) > 0);
	set_stages[0] = set_stage_one;
	set_stages[1] = set_stage_two;
	set_stages[2] = set_stage_three;
	set_backups[0] = set_backup_one;
	set_backups[1] = set_backup_two;
	set_backups[2] = set_backup_three;
	set_destinations[0] = set_first;
	set_destinations[1] = set_second;
	set_destinations[2] = set_third;
	assert(snprintf(config_destination, sizeof(config_destination), "%s/dnsmasq.conf", directory) > 0);
	assert(snprintf(config_template, sizeof(config_template), "%s/dnsmasq.conf.XXXXXX", directory) > 0);
	assert(snprintf(config_fail_template, sizeof(config_fail_template), "%s/dnsmasq.conf.failed.XXXXXX", directory) > 0);
	assert(snprintf(resolv_destination, sizeof(resolv_destination), "%s/resolv.conf", directory) > 0);
	assert(snprintf(resolv_same_template, sizeof(resolv_same_template), "%s/resolv.same.XXXXXX", directory) > 0);
	assert(snprintf(resolv_diff_template, sizeof(resolv_diff_template), "%s/resolv.diff.XXXXXX", directory) > 0);
	assert(snprintf(resolv_symlink, sizeof(resolv_symlink), "%s/resolv.link", directory) > 0);
	assert(snprintf(resolv_link_template, sizeof(resolv_link_template), "%s/resolv.link.XXXXXX", directory) > 0);

	fp = fopen(sentinel, "w");
	assert(fp != NULL);
	assert(fputs("preserve-me\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(chmod(sentinel, 0644) == 0);

	/* Safe input is opened and checked on the file descriptor actually read. */
	fp = dnsmasq_file_open_safe_read(sentinel);
	assert(fp != NULL);
	{
		char line[32];
		assert(fgets(line, sizeof(line), fp) != NULL);
		assert(strcmp(line, "preserve-me\n") == 0);
	}
	assert(fclose(fp) == 0);

	/* Publishing over a planted symlink replaces the link, not its target. */
	assert(symlink(sentinel, destination) == 0);
	assert(dnsmasq_file_is_safe_regular(destination) == -1);
	fp = dnsmasq_file_open(template);
	assert(fp != NULL);
	assert(snprintf(staged, sizeof(staged), "%s", template) > 0);
	assert(stat(staged, &st) == 0);
	assert((st.st_mode & 0777) == 0600);
	assert(fputs("server=/example.test/192.0.2.53\n", fp) >= 0);
	assert(dnsmasq_file_publish(fp, template, destination, 0644) == 0);
	assert(template[0] == '\0');
	assert(lstat(destination, &st) == 0 && S_ISREG(st.st_mode));
	assert(st.st_uid == geteuid() && st.st_nlink == 1);
	assert((st.st_mode & 0777) == 0644);
	assert(dnsmasq_file_is_safe_regular(destination) == 0);
	assert(chmod(destination, 0666) == 0);
	assert(dnsmasq_file_is_safe_regular(destination) == -1);
	assert(chmod(destination, 0644) == 0);
	read_exact(destination, "server=/example.test/192.0.2.53\n");
	read_exact(sentinel, "preserve-me\n");

	/* Resolver comparison must distinguish equal/different content and reject links. */
	fp = fopen(resolv_destination, "w");
	assert(fp != NULL);
	assert(fputs("nameserver 127.0.0.1\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(chmod(resolv_destination, 0644) == 0);
	fp = dnsmasq_file_open(resolv_same_template);
	assert(fp != NULL);
	assert(fputs("nameserver 127.0.0.1\n", fp) >= 0);
	assert(fflush(fp) == 0);
	assert(dnsmasq_file_contents_equal(resolv_destination, resolv_same_template) == 1);
	assert(fclose(fp) == 0);
	assert(dnsmasq_file_discard(resolv_same_template) == 0);

	fp = dnsmasq_file_open(resolv_diff_template);
	assert(fp != NULL);
	assert(fputs("nameserver 192.0.2.53\n", fp) >= 0);
	assert(fflush(fp) == 0);
	assert(dnsmasq_file_contents_equal(resolv_destination, resolv_diff_template) == 0);
	assert(fclose(fp) == 0);
	assert(dnsmasq_file_discard(resolv_diff_template) == 0);

	assert(symlink(sentinel, resolv_symlink) == 0);
	fp = dnsmasq_file_open(resolv_link_template);
	assert(fp != NULL);
	assert(fputs("preserve-me\n", fp) >= 0);
	assert(fflush(fp) == 0);
	assert(dnsmasq_file_contents_equal(resolv_symlink, resolv_link_template) == 0);
	assert(fclose(fp) == 0);
	assert(dnsmasq_file_discard(resolv_link_template) == 0);
	read_exact(sentinel, "preserve-me\n");

	/* Finalization errors retain errno, unlink staging and preserve old output. */
	fp = dnsmasq_file_open(fail_template);
	assert(fp != NULL);
	assert(snprintf(fail_path, sizeof(fail_path), "%s", fail_template) > 0);
	assert(fputs("partial\n", fp) >= 0);
	assert(close(fileno(fp)) == 0);
	errno = 0;
	assert(dnsmasq_file_publish(fp, fail_template, destination, 0644) == -1);
	saved_errno = errno;
	assert(saved_errno != 0);
	assert(fail_template[0] == '\0');
	assert(access(fail_path, F_OK) != 0 && errno == ENOENT);
	read_exact(destination, "server=/example.test/192.0.2.53\n");

	/* A rename failure must discard staging and preserve the last-good file. */
	fp = dnsmasq_file_open(rename_fail_template);
	assert(fp != NULL);
	assert(snprintf(rename_fail_path, sizeof(rename_fail_path), "%s", rename_fail_template) > 0);
	assert(fputs("replacement\n", fp) >= 0);
	errno = 0;
	assert(dnsmasq_file_publish(fp, rename_fail_template, directory, 0644) == -1);
	saved_errno = errno;
	assert(saved_errno != 0);
	assert(rename_fail_template[0] == '\0');
	assert(access(rename_fail_path, F_OK) != 0 && errno == ENOENT);
	read_exact(destination, "server=/example.test/192.0.2.53\n");

	/* The main service config is also staged and atomically published. */
	fp = fopen(config_destination, "w");
	assert(fp != NULL);
	assert(fputs("last-known-good\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(chmod(config_destination, 0644) == 0);
	fp = dnsmasq_file_open(config_template);
	assert(fp != NULL);
	assert(fputs("generated-config\n", fp) >= 0);
	assert(dnsmasq_file_publish(fp, config_template, config_destination, 0644) == 0);
	read_exact(config_destination, "generated-config\n");
	assert(stat(config_destination, &st) == 0 && (st.st_mode & 0777) == 0644);

	/* A late flush/close error must preserve the previous service config. */
	fp = dnsmasq_file_open(config_fail_template);
	assert(fp != NULL);
	assert(snprintf(config_fail_path, sizeof(config_fail_path), "%s", config_fail_template) > 0);
	assert(fputs("partial-config\n", fp) >= 0);
	assert(close(fileno(fp)) == 0);
	errno = 0;
	assert(dnsmasq_file_publish(fp, config_fail_template, config_destination, 0644) == -1);
	saved_errno = errno;
	assert(saved_errno != 0);
	assert(config_fail_template[0] == '\0');
	assert(access(config_fail_path, F_OK) != 0 && errno == ENOENT);
	read_exact(config_destination, "generated-config\n");

	
	/* A failed second publish restores the original first destination. */
	fp = fopen(pair_first, "w");
	assert(fp != NULL && fputs("old-hostname\n", fp) >= 0 && fclose(fp) == 0);
	fp = dnsmasq_file_open(pair_first_stage);
	pair_fp = dnsmasq_file_open(pair_second_stage);
	assert(fp != NULL && pair_fp != NULL);
	assert(fputs("new-hostname\n", fp) >= 0 && fputs("new-hosts\n", pair_fp) >= 0);
	errno = 0;
	assert(dnsmasq_file_publish_pair(fp, pair_first_stage, pair_first, pair_first_backup,
					 pair_fp, pair_second_stage, pair_bad_second,
					 pair_second_backup, 0644) == -1);
	assert(errno == ENOENT);
	read_exact(pair_first, "old-hostname\n");
	assert(access(pair_bad_second, F_OK) != 0 && errno == ENOENT);
	assert(pair_first_stage[0] == '\0' && pair_second_stage[0] == '\0');
	assert(pair_first_backup[0] == '\0' && pair_second_backup[0] == '\0');

	/* An absent first destination is removed when second publication fails. */
	assert(snprintf(pair_absent_stage, sizeof(pair_absent_stage), "%s/pair.absent.XXXXXX", directory) > 0);
	assert(snprintf(pair_second_stage, sizeof(pair_second_stage), "%s/pair.hosts.fail.XXXXXX", directory) > 0);
	assert(snprintf(pair_absent_backup, sizeof(pair_absent_backup), "%s/pair.absent.rollback.XXXXXX", directory) > 0);
	assert(snprintf(pair_second_backup, sizeof(pair_second_backup), "%s/pair.hosts.fail.rollback.XXXXXX", directory) > 0);
	fp = dnsmasq_file_open(pair_absent_stage);
	pair_fp = dnsmasq_file_open(pair_second_stage);
	assert(fp != NULL && pair_fp != NULL);
	assert(fputs("new-absent\n", fp) >= 0 && fputs("new-hosts\n", pair_fp) >= 0);
	assert(dnsmasq_file_publish_pair(fp, pair_absent_stage, pair_absent_first,
					 pair_absent_backup, pair_fp, pair_second_stage,
					 pair_bad_second, pair_second_backup, 0644) == -1);
	assert(errno == ENOENT);
	assert(access(pair_absent_first, F_OK) != 0 && errno == ENOENT);
	assert(pair_absent_backup[0] == '\0' && pair_second_backup[0] == '\0');

	assert(unlink(pair_first) == 0);
	/* Restore the symlink entry itself, never its referenced target. */
	fp = fopen(pair_sentinel, "w");
	assert(fp != NULL && fputs("sentinel\n", fp) >= 0 && fclose(fp) == 0);
	assert(symlink(pair_sentinel, pair_first) == 0);
	assert(snprintf(pair_first_stage, sizeof(pair_first_stage), "%s/pair.hostname.XXXXXX", directory) > 0);
	assert(snprintf(pair_second_stage, sizeof(pair_second_stage), "%s/pair.hosts.symlink.XXXXXX", directory) > 0);
	assert(snprintf(pair_first_backup, sizeof(pair_first_backup), "%s/pair.hostname.rollback.XXXXXX", directory) > 0);
	assert(snprintf(pair_second_backup, sizeof(pair_second_backup), "%s/pair.hosts.symlink.rollback.XXXXXX", directory) > 0);
	fp = dnsmasq_file_open(pair_first_stage);
	pair_fp = dnsmasq_file_open(pair_second_stage);
	assert(fp != NULL && pair_fp != NULL);
	assert(fputs("new-hostname\n", fp) >= 0 && fputs("new-hosts\n", pair_fp) >= 0);
	assert(dnsmasq_file_publish_pair(fp, pair_first_stage, pair_first, pair_first_backup,
					 pair_fp, pair_second_stage, pair_bad_second,
					 pair_second_backup, 0644) == -1);
	assert(errno == ENOENT);
	assert(lstat(pair_first, &pair_stat) == 0 && S_ISLNK(pair_stat.st_mode));
	read_exact(pair_sentinel, "sentinel\n");
	assert(unlink(pair_first) == 0 && unlink(pair_sentinel) == 0);

	/* Successful publication replaces both files and removes snapshots. */
	fp = fopen(pair_first, "w");
	assert(fp != NULL && fputs("old-hostname\n", fp) >= 0 && fclose(fp) == 0);
	fp = fopen(pair_second, "w");
	assert(fp != NULL && fputs("old-hosts\n", fp) >= 0 && fclose(fp) == 0);
	assert(snprintf(pair_first_stage, sizeof(pair_first_stage), "%s/pair.hostname.XXXXXX", directory) > 0);
	assert(snprintf(pair_second_stage, sizeof(pair_second_stage), "%s/pair.hosts.XXXXXX", directory) > 0);
	assert(snprintf(pair_first_backup, sizeof(pair_first_backup), "%s/pair.hostname.rollback.XXXXXX", directory) > 0);
	assert(snprintf(pair_second_backup, sizeof(pair_second_backup), "%s/pair.hosts.rollback.XXXXXX", directory) > 0);
	fp = dnsmasq_file_open(pair_first_stage);
	pair_fp = dnsmasq_file_open(pair_second_stage);
	assert(fp != NULL && pair_fp != NULL);
	assert(fputs("committed-hostname\n", fp) >= 0 && fputs("committed-hosts\n", pair_fp) >= 0);
	assert(dnsmasq_file_publish_pair(fp, pair_first_stage, pair_first, pair_first_backup,
					 pair_fp, pair_second_stage, pair_second, pair_second_backup,
					 0644) == 0);
	assert(pair_first_backup[0] == '\0' && pair_second_backup[0] == '\0');
	read_exact(pair_first, "committed-hostname\n");
	read_exact(pair_second, "committed-hosts\n");
	assert(unlink(pair_first) == 0 && unlink(pair_second) == 0);


	/* A third rename failure rolls back both earlier replacements in reverse order. */
	fp = fopen(set_first, "w");
	assert(fp != NULL && fputs("old-one\n", fp) >= 0 && fclose(fp) == 0);
	fp = fopen(set_second, "w");
	assert(fp != NULL && fputs("old-two\n", fp) >= 0 && fclose(fp) == 0);
	set_destinations[2] = set_bad_third;
	set_files[0] = dnsmasq_file_open(set_stages[0]);
	set_files[1] = dnsmasq_file_open(set_stages[1]);
	set_files[2] = dnsmasq_file_open(set_stages[2]);
	assert(set_files[0] != NULL && set_files[1] != NULL && set_files[2] != NULL);
	assert(fputs("new-one\n", set_files[0]) >= 0);
	assert(fputs("new-two\n", set_files[1]) >= 0);
	assert(fputs("new-three\n", set_files[2]) >= 0);
	errno = 0;
	assert(dnsmasq_file_publish_set(set_files, set_stages, set_destinations,
					set_backups, 3, 0644) == -1);
	assert(errno == ENOENT);
	read_exact(set_first, "old-one\n");
	read_exact(set_second, "old-two\n");
	assert(access(set_bad_third, F_OK) != 0 && errno == ENOENT);
	assert(set_stage_one[0] == '\0' && set_stage_two[0] == '\0' && set_stage_three[0] == '\0');
	assert(set_backup_one[0] == '\0' && set_backup_two[0] == '\0' && set_backup_three[0] == '\0');

	/* The same helper commits all three complete outputs when every rename works. */
	set_destinations[2] = set_third;
	fp = fopen(set_third, "w");
	assert(fp != NULL && fputs("old-three\n", fp) >= 0 && fclose(fp) == 0);
	assert(snprintf(set_stage_one, sizeof(set_stage_one), "%s/set-one.XXXXXX", directory) > 0);
	assert(snprintf(set_stage_two, sizeof(set_stage_two), "%s/set-two.XXXXXX", directory) > 0);
	assert(snprintf(set_stage_three, sizeof(set_stage_three), "%s/set-three.XXXXXX", directory) > 0);
	assert(snprintf(set_backup_one, sizeof(set_backup_one), "%s/set-one.rollback.XXXXXX", directory) > 0);
	assert(snprintf(set_backup_two, sizeof(set_backup_two), "%s/set-two.rollback.XXXXXX", directory) > 0);
	assert(snprintf(set_backup_three, sizeof(set_backup_three), "%s/set-three.rollback.XXXXXX", directory) > 0);
	set_files[0] = dnsmasq_file_open(set_stages[0]);
	set_files[1] = dnsmasq_file_open(set_stages[1]);
	set_files[2] = dnsmasq_file_open(set_stages[2]);
	assert(set_files[0] != NULL && set_files[1] != NULL && set_files[2] != NULL);
	assert(fputs("commit-one\n", set_files[0]) >= 0);
	assert(fputs("commit-two\n", set_files[1]) >= 0);
	assert(fputs("commit-three\n", set_files[2]) >= 0);
	assert(dnsmasq_file_publish_set(set_files, set_stages, set_destinations,
					set_backups, 3, 0644) == 0);
	assert(set_stage_one[0] == '\0' && set_stage_two[0] == '\0' && set_stage_three[0] == '\0');
	assert(set_backup_one[0] == '\0' && set_backup_two[0] == '\0' && set_backup_three[0] == '\0');
	read_exact(set_first, "commit-one\n");
	read_exact(set_second, "commit-two\n");
	read_exact(set_third, "commit-three\n");
	assert(unlink(set_first) == 0 && unlink(set_second) == 0 && unlink(set_third) == 0);

/* Lease file initialization must reject a symlink without touching target. */
	assert(symlink(sentinel, symlink_lease) == 0);
	errno = 0;
	assert(dnsmasq_file_open_safe_read(symlink_lease) == NULL);
	assert(errno == ELOOP || errno == EMLINK || errno == EACCES);
	assert(dnsmasq_lease_file_ensure(symlink_lease) == -1);
	assert(lstat(symlink_lease, &st) == 0 && S_ISLNK(st.st_mode));
	read_exact(sentinel, "preserve-me\n");

	/* Existing regular lease files are preserved and normalized to safe mode. */
	fp = fopen(lease_path, "w");
	assert(fp != NULL);
	assert(fputs("lease-record\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(chmod(lease_path, 0666) == 0);
	assert(dnsmasq_lease_file_ensure(lease_path) == 0);
	assert(stat(lease_path, &st) == 0 && S_ISREG(st.st_mode));
	assert(st.st_uid == geteuid() && st.st_nlink == 1);
	assert((st.st_mode & 0777) == 0644);
	read_exact(lease_path, "lease-record\n");

	/* A stricter pre-existing mode must not be widened during validation. */
	fp = fopen(private_lease, "w");
	assert(fp != NULL);
	assert(fputs("private-lease\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(chmod(private_lease, 0600) == 0);
	assert(dnsmasq_lease_file_ensure(private_lease) == 0);
	assert(stat(private_lease, &st) == 0 && (st.st_mode & 0777) == 0600);
	read_exact(private_lease, "private-lease\n");

	fp = fopen(unsafe_read, "w");
	assert(fp != NULL);
	assert(fputs("untrusted\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(chmod(unsafe_read, 0666) == 0);
	errno = 0;
	assert(dnsmasq_file_open_safe_read(unsafe_read) == NULL);
	assert(errno == EPERM);
	assert(chmod(unsafe_read, 0600) == 0);
	fp = dnsmasq_file_open_safe_read(unsafe_read);
	assert(fp != NULL);
	assert(fclose(fp) == 0);

	/* Do not chmod a hard-linked victim or block while opening a FIFO. */
	assert(link(sentinel, hardlink_lease) == 0);
	errno = 0;
	assert(dnsmasq_file_open_safe_read(hardlink_lease) == NULL);
	assert(errno == EMLINK);
	assert(dnsmasq_file_is_safe_regular(hardlink_lease) == -1);
	assert(dnsmasq_lease_file_ensure(hardlink_lease) == -1);
	assert(stat(sentinel, &st) == 0 && (st.st_mode & 0777) == 0644);
	assert(mkfifo(fifo_lease, 0600) == 0);
	errno = 0;
	assert(dnsmasq_file_open_safe_read(fifo_lease) == NULL);
	assert(errno == EINVAL);
	assert(dnsmasq_file_is_safe_regular(fifo_lease) == -1);
	assert(dnsmasq_lease_file_ensure(fifo_lease) == -1);
	assert(lstat(fifo_lease, &st) == 0 && S_ISFIFO(st.st_mode));
	read_exact(sentinel, "preserve-me\n");

	assert(unlink(hardlink_lease) == 0);
	assert(unlink(fifo_lease) == 0);
	assert(unlink(symlink_lease) == 0);
	assert(unlink(private_lease) == 0);
	assert(unlink(unsafe_read) == 0);
	assert(unlink(lease_path) == 0);
	assert(unlink(destination) == 0);
	assert(unlink(config_destination) == 0);
	assert(unlink(resolv_symlink) == 0);
	assert(unlink(resolv_destination) == 0);
	assert(unlink(sentinel) == 0);
	assert(rmdir(directory) == 0);
	return 0;
}

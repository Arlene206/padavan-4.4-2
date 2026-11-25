#ifndef FIREFLY_CONFIG_FILE_H
#define FIREFLY_CONFIG_FILE_H

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct firefly_config_values {
	const char *link_path;
	const char *username;
	const char *password;
};

static int
firefly_config_printf(FILE *stream, const char *format, ...)
{
	va_list arguments;
	int result;

	errno = 0;
	va_start(arguments, format);
	result = vfprintf(stream, format, arguments);
	va_end(arguments);
	if (result < 0)
		return errno ? errno : EIO;

	return 0;
}

/* Match an INI key name, not merely a same-prefix custom directive. */
static int
firefly_config_line_has_key(const char *line, const char *key)
{
	size_t key_length;

	if (!line || !key || !key[0])
		return 0;
	while (*line == ' ' || *line == '\t')
		line++;
	key_length = strlen(key);
	if (strncmp(line, key, key_length) != 0)
		return 0;
	line += key_length;
	while (*line == ' ' || *line == '\t')
		line++;
	return *line == '=';
}


static int
firefly_config_write_existing(FILE *output, FILE *source,
			      const struct firefly_config_values *values)
{
	char line[512];
	int status;

	while (fgets(line, sizeof(line), source) != NULL) {
		if (firefly_config_line_has_key(line, "web_root"))
			status = firefly_config_printf(output, "web_root = %s\n",
						       "/usr/share/mt-daapd");
		else if (firefly_config_line_has_key(line, "port"))
			status = firefly_config_printf(output, "port = %d\n", 3689);
		else if (firefly_config_line_has_key(line, "runas"))
			status = firefly_config_printf(output, "runas = %s\n",
						       values->username);
		else if (firefly_config_line_has_key(line, "db_type"))
			status = firefly_config_printf(output, "db_type = %s\n",
						       "sqlite3");
		else if (firefly_config_line_has_key(line, "db_parms"))
			status = firefly_config_printf(output, "db_parms = %s\n",
						       values->link_path);
		else if (firefly_config_line_has_key(line, "plugin_dir"))
			status = firefly_config_printf(output, "plugin_dir = %s\n",
						       "/usr/lib/mt-daapd");
		else
			status = firefly_config_printf(output, "%s", line);

		if (status != 0)
			return status;
	}
	if (ferror(source))
		return errno ? errno : EIO;

	return 0;
}

static int
firefly_config_write_defaults(FILE *output,
			      const struct firefly_config_values *values)
{
	return firefly_config_printf(output,
		"[general]\n"
		"web_root = %s\n"
		"port = %d\n"
		"runas = %s\n"
		"admin_pw = %s\n"
		"db_type = %s\n"
		"db_parms = %s\n"
		"logfile = %s/mt-daapd.log\n"
		"servername = %s\n"
		"mp3_dir = %s\n"
		"extensions = %s\n"
		"rescan_interval = %d\n"
		"always_scan = %d\n"
		"scan_type = %d\n"
		"debuglevel = %d\n\n"
		"[scanning]\n"
		"skip_first = %d\n"
		"process_playlists = %d\n"
		"process_itunes = %d\n"
		"process_m3u = %d\n"
		"mp3_tag_codepage = %s\n\n"
		"[plugins]\n"
		"plugin_dir = %s\n\n",
		"/usr/share/mt-daapd", 3689, values->username,
		values->password, "sqlite3", values->link_path,
		values->link_path, "Firefly on %h", "/media",
		".mp3,.m4a,.m4p,.flac,.alac", 300, 0, 0, 0,
		1, 1, 1, 1, "WINDOWS-1251", "/usr/lib/mt-daapd");
}

static int
firefly_config_text_is_safe(const char *value)
{
	const unsigned char *cursor = (const unsigned char *)value;

	if (!cursor)
		return 0;
	while (*cursor) {
		if (*cursor < 0x20 || *cursor == 0x7f)
			return 0;
		cursor++;
	}
	return 1;
}


/* Publish a complete config; never replace the last good file on failure. */
static int
firefly_config_publish(const char *path, const char *source_path,
		       const char *link_path, const char *username,
		       const char *password)
{
	static const char suffix[] = ".tmp.XXXXXX";
	struct firefly_config_values values;
	struct stat source_info;
	char *temporary_path = NULL;
	size_t path_length;
	FILE *source = NULL;
	FILE *output = NULL;
	int source_fd = -1;
	int fd = -1;
	int status = 0;
	int sync_status;

	if (path == NULL || path[0] == '\0' ||
	    !firefly_config_text_is_safe(link_path) ||
	    !firefly_config_text_is_safe(username) ||
	    !firefly_config_text_is_safe(password))
		return EINVAL;

	values.link_path = link_path;
	values.username = username;
	values.password = password;

	if (source_path != NULL && source_path[0] != '\0') {
		/*
		 * A corrupt FIFO at the persisted config path must not block
		 * service startup forever. Open nonblocking, pin the object with
		 * fstat(), and only read a regular file. Symlinks to regular files
		 * remain compatible because open() follows and pins their target.
		 */
		errno = 0;
		source_fd = open(source_path, O_RDONLY | O_NONBLOCK | O_NOCTTY);
		if (source_fd < 0) {
			if (errno != ENOENT)
				return errno ? errno : EIO;
		} else {
			if (fstat(source_fd, &source_info) != 0) {
				status = errno ? errno : EIO;
				goto out;
			}
			if (!S_ISREG(source_info.st_mode)) {
				status = EINVAL;
				goto out;
			}
			source = fdopen(source_fd, "r");
			if (source == NULL) {
				status = errno ? errno : EIO;
				goto out;
			}
			source_fd = -1;
		}
	}

	path_length = strlen(path);
	if (path_length > (size_t)-1 - sizeof(suffix)) {
		status = ENAMETOOLONG;
		goto out;
	}
	temporary_path = (char *)malloc(path_length + sizeof(suffix));
	if (temporary_path == NULL) {
		status = ENOMEM;
		goto out;
	}
	memcpy(temporary_path, path, path_length);
	memcpy(temporary_path + path_length, suffix, sizeof(suffix));

	fd = mkstemp(temporary_path);
	if (fd < 0) {
		status = errno ? errno : EIO;
		goto out;
	}
	output = fdopen(fd, "w");
	if (output == NULL) {
		status = errno ? errno : EIO;
		close(fd);
		fd = -1;
		goto out;
	}
	fd = -1;

	if (source != NULL)
		status = firefly_config_write_existing(output, source, &values);
	else
		status = firefly_config_write_defaults(output, &values);

	if (source != NULL) {
		if (fclose(source) != 0 && status == 0)
			status = errno ? errno : EIO;
		source = NULL;
	}
	if (status == 0) {
		errno = 0;
		if (fflush(output) != 0)
			status = errno ? errno : EIO;
	}
	if (status == 0) {
		do {
			errno = 0;
			sync_status = fsync(fileno(output));
		} while (sync_status != 0 && errno == EINTR);
		if (sync_status != 0)
			status = errno ? errno : EIO;
	}
	if (fclose(output) != 0 && status == 0)
		status = errno ? errno : EIO;
	output = NULL;

	if (status == 0) {
		errno = 0;
		if (rename(temporary_path, path) != 0)
			status = errno ? errno : EIO;
	}
	if (status != 0)
		unlink(temporary_path);

out:
	if (source != NULL)
		fclose(source);
	if (source_fd >= 0)
		close(source_fd);
	if (output != NULL)
		fclose(output);
	if (fd >= 0)
		close(fd);
	if (temporary_path != NULL) {
		if (status != 0)
			unlink(temporary_path);
		free(temporary_path);
	}
	return status;
}

#endif /* FIREFLY_CONFIG_FILE_H */

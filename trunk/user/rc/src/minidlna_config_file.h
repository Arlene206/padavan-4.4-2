#ifndef MINIDLNA_CONFIG_FILE_H
#define MINIDLNA_CONFIG_FILE_H

#include "config_file_sync.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct minidlna_config_values {
	const char *interface_name;
	int notify_interval;
	const char *media_source1;
	const char *media_source2;
	const char *media_source3;
	int root_container;
	int force_sort;
	const char *friendly_name;
	const char *database_dir;
};

static int
minidlna_config_printf(FILE *stream, const char *format, ...)
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

static int
minidlna_config_write(FILE *stream,
		      const struct minidlna_config_values *values)
{
	const char *root_container = NULL;
	int status;

	if (stream == NULL || values == NULL ||
	    values->interface_name == NULL ||
	    values->media_source1 == NULL ||
	    values->media_source2 == NULL ||
	    values->media_source3 == NULL ||
	    values->friendly_name == NULL ||
	    values->database_dir == NULL)
		return EINVAL;

	status = minidlna_config_printf(stream, "port=%d\n", 8200);
	if (status == 0)
		status = minidlna_config_printf(stream, "network_interface=%s\n",
						values->interface_name);
	if (status == 0)
		status = minidlna_config_printf(stream, "notify_interval=%d\n",
						values->notify_interval);
	if (status != 0)
		return status;

	if (*values->media_source1) {
		status = minidlna_config_printf(stream, "media_dir=%s\n",
						values->media_source1);
		if (status != 0)
			return status;
	}
	if (*values->media_source2) {
		status = minidlna_config_printf(stream, "media_dir=%s\n",
						values->media_source2);
		if (status != 0)
			return status;
	}
	if (*values->media_source3) {
		status = minidlna_config_printf(stream, "media_dir=%s\n",
						values->media_source3);
		if (status != 0)
			return status;
	}

	status = minidlna_config_printf(stream, "merge_media_dirs=%s\n", "no");
	if (status != 0)
		return status;

	switch (values->root_container) {
	case 1:
		root_container = "B";
		break;
	case 2:
		root_container = "M";
		break;
	case 3:
		root_container = "V";
		break;
	case 4:
		root_container = "P";
		break;
	default:
		break;
	}
	if (root_container != NULL) {
		status = minidlna_config_printf(stream, "root_container=%s\n",
						root_container);
		if (status != 0)
			return status;
	}
	if (values->force_sort > 0) {
		status = minidlna_config_printf(stream,
			"force_sort_criteria=%s\n",
			"+upnp:class,+upnp:originalTrackNumber,+dc:title");
		if (status != 0)
			return status;
	}

	status = minidlna_config_printf(stream, "friendly_name=%s\n",
					values->friendly_name);
	if (status == 0)
		status = minidlna_config_printf(stream, "db_dir=%s\n",
						values->database_dir);
	if (status == 0)
		status = minidlna_config_printf(stream, "log_dir=%s\n",
						values->database_dir);
	if (status == 0)
		status = minidlna_config_printf(stream,
			"album_art_names=%s\n",
			"Cover.jpg/cover.jpg/AlbumArtSmall.jpg/albumartsmall.jpg/AlbumArt.jpg/albumart.jpg/Album.jpg/album.jpg/Folder.jpg/folder.jpg/Thumb.jpg/thumb.jpg");
	if (status == 0)
		status = minidlna_config_printf(stream, "wide_links=%s\n", "yes");
	if (status == 0)
		status = minidlna_config_printf(stream, "inotify=%s\n", "yes");
	if (status == 0)
		status = minidlna_config_printf(stream, "enable_tivo=%s\n", "no");
	if (status == 0)
		status = minidlna_config_printf(stream, "strict_dlna=%s\n", "no");
	if (status == 0)
		status = minidlna_config_printf(stream, "model_number=%d\n", 1);

	return status;
}

static int
minidlna_config_text_is_safe(const char *value)
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


static int
minidlna_config_values_are_safe(const struct minidlna_config_values *values)
{
	return values != NULL &&
		minidlna_config_text_is_safe(values->interface_name) &&
		minidlna_config_text_is_safe(values->media_source1) &&
		minidlna_config_text_is_safe(values->media_source2) &&
		minidlna_config_text_is_safe(values->media_source3) &&
		minidlna_config_text_is_safe(values->friendly_name) &&
		minidlna_config_text_is_safe(values->database_dir);
}


/* Replace the active config only after a complete, durable staged write. */
static int
minidlna_config_publish(const char *path,
			const struct minidlna_config_values *values)
{
	static const char suffix[] = ".tmp.XXXXXX";
	char *temporary_path = NULL;
	size_t path_length;
	FILE *output = NULL;
	int fd = -1;
	int status = 0;
	int sync_status;

	if (path == NULL || path[0] == '\0' ||
	    !minidlna_config_values_are_safe(values))
		return EINVAL;

	path_length = strlen(path);
	if (path_length > (size_t)-1 - sizeof(suffix))
		return ENAMETOOLONG;
	temporary_path = (char *)malloc(path_length + sizeof(suffix));
	if (temporary_path == NULL)
		return ENOMEM;
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

	status = minidlna_config_write(output, values);
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
		else if (config_file_sync_parent(path) != 0)
			status = errno ? errno : EIO;
	}

out:
	if (output != NULL)
		fclose(output);
	if (fd >= 0)
		close(fd);
	if (status != 0)
		unlink(temporary_path);
	free(temporary_path);
	return status;
}

#endif /* MINIDLNA_CONFIG_FILE_H */

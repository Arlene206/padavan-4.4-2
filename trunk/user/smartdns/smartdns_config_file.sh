#!/bin/sh
# Publish a de-duplicated SmartDNS configuration without truncating the
# active file until the complete replacement has been written successfully.
# Validate a single SmartDNS config argument. Preserve printable UTF-8
# labels, but reject token separators, escapes, comments and control bytes.
smartdns_config_token_valid()
{
	[ "$#" -eq 1 ] || return 2
	[ -n "$1" ] || return 1
	case "$1" in
		*[[:space:]]*|*[[:cntrl:]]*|*'#'*|*';'*|*'='*|*'\'*|*'"'*|*"'"*)
			return 1
			;;
	esac
	return 0
}

smartdns_config_publish_deduplicated()
{
	[ "$#" -eq 2 ] || return 2
	_smartdns_source=$1
	_smartdns_destination=$2
	[ -n "$_smartdns_source" ] && [ -n "$_smartdns_destination" ] || return 2

	if [ -L "$_smartdns_source" ] || [ ! -f "$_smartdns_source" ] ||
	   [ ! -r "$_smartdns_source" ]; then
		return 1
	fi
	if [ -L "$_smartdns_destination" ] ||
	   { [ -e "$_smartdns_destination" ] && [ ! -f "$_smartdns_destination" ]; }; then
		return 1
	fi

	case "$_smartdns_destination" in
		*/*) _smartdns_parent=${_smartdns_destination%/*} ;;
		*) _smartdns_parent=. ;;
	esac
	[ -n "$_smartdns_parent" ] || _smartdns_parent=/
	[ -d "$_smartdns_parent" ] && [ ! -L "$_smartdns_parent" ] || return 1

	_smartdns_stage=$(mktemp "${_smartdns_destination}.tmp.XXXXXX") || return 1
	if awk '!seen[$0]++' "$_smartdns_source" > "$_smartdns_stage"; then
		:
	else
		_smartdns_status=$?
		rm -f "$_smartdns_stage"
		return "$_smartdns_status"
	fi
	if chmod 0644 "$_smartdns_stage"; then
		:
	else
		_smartdns_status=$?
		rm -f "$_smartdns_stage"
		return "$_smartdns_status"
	fi

	# Recheck the final component immediately before rename. rename replaces a
	# raced symlink itself rather than following its target.
	if [ -L "$_smartdns_destination" ] ||
	   { [ -e "$_smartdns_destination" ] && [ ! -f "$_smartdns_destination" ]; }; then
		rm -f "$_smartdns_stage"
		return 1
	fi
	if mv -f "$_smartdns_stage" "$_smartdns_destination"; then
		return 0
	else
		_smartdns_status=$?
		rm -f "$_smartdns_stage"
		return "$_smartdns_status"
	fi
}

# Append non-comment, non-empty lines from an optional user config file.
# Missing files are intentionally treated as empty, matching legacy behavior.
smartdns_config_append_filtered()
{
	[ "$#" -eq 2 ] || return 2
	_smartdns_append_source=$1
	_smartdns_append_destination=$2
	[ -n "$_smartdns_append_source" ] && [ -n "$_smartdns_append_destination" ] || return 2

	if [ ! -e "$_smartdns_append_source" ] && [ ! -L "$_smartdns_append_source" ]; then
		return 0
	fi
	[ -f "$_smartdns_append_source" ] && [ -r "$_smartdns_append_source" ] || return 1
	[ -f "$_smartdns_append_destination" ] &&
		[ ! -L "$_smartdns_append_destination" ] &&
		[ -w "$_smartdns_append_destination" ] || return 1

	awk '$0 !~ /^#/ && $0 != "" { print }' "$_smartdns_append_source" >> "$_smartdns_append_destination"
}

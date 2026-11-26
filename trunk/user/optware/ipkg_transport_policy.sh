#!/bin/sh

# ipkg may execute package-maintainer scripts as root. Its downloader must
# never fall back to plaintext HTTP/FTP or silently accept an unknown scheme.
ipkg_transport_require_url()
{
    case "$1" in
        https://*|file:/*)
            return 0
            ;;
        http://*|ftp://*)
            echo "ipkg: refusing insecure package URL: $1" >&2
            return 1
            ;;
        *)
            echo "ipkg: refusing unsupported package URL: $1" >&2
            return 1
            ;;
    esac
}

# The legacy Oleg feed is HTTP-only. Reject it before any package metadata or
# package payload can be downloaded and installed with root privileges.
ipkg_transport_config_is_secure()
{
    [ "$#" -eq 1 ] || return 2
    iptc_config=$1
    [ -f "$iptc_config" ] && [ ! -L "$iptc_config" ] && [ -r "$iptc_config" ] || return 1

    awk '
        /^[[:space:]]*#/ || /^[[:space:]]*$/ { next }
        $1 == "src" {
            source_count++
            if (NF != 3 || ($3 !~ /^https:\/\// && $3 !~ /^file:\//)) {
                print "ipkg: refusing non-HTTPS or malformed source: " $0 > "/dev/stderr"
                invalid = 1
            }
        }
        END {
            if (source_count == 0 || invalid)
                exit 1
        }
    ' "$iptc_config"
}

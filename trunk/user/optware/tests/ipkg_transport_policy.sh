#!/bin/sh
set -eu

test_dir=$(mktemp -d /tmp/ipkg-transport-test.XXXXXX)
trap 'rm -rf "$test_dir"' 0
trap 'exit 1' HUP INT TERM

. "$(dirname "$0")/../ipkg_transport_policy.sh"

ipkg_transport_require_url "https://packages.example.invalid/repo/Packages"
ipkg_transport_require_url "file:///tmp/Packages"
if ipkg_transport_require_url "http://packages.example.invalid/repo/Packages"; then
    echo "HTTP package URL was accepted" >&2
    exit 1
fi
if ipkg_transport_require_url "ftp://packages.example.invalid/repo/Packages"; then
    echo "FTP package URL was accepted" >&2
    exit 1
fi
if ipkg_transport_require_url "gopher://packages.example.invalid/Packages"; then
    echo "unknown package URL scheme was accepted" >&2
    exit 1
fi
echo "ipkg permits HTTPS/local files and rejects HTTP/FTP/unknown schemes: PASS"

cat > "$test_dir/secure.conf" <<'EOF'
# HTTPS repositories are eligible for privileged package operations.
src optware https://packages.example.invalid/feeds/stable
src local file:///opt/packages
dest root /
EOF
ipkg_transport_config_is_secure "$test_dir/secure.conf"
echo "ipkg accepts HTTPS and local-file source configuration: PASS"

cat > "$test_dir/http.conf" <<'EOF'
src optware http://ipkg.nslu2-linux.org/feeds/optware/oleg/cross/stable
dest root /
EOF
if ipkg_transport_config_is_secure "$test_dir/http.conf"; then
    echo "legacy HTTP-only Optware feed was accepted" >&2
    exit 1
fi
cat > "$test_dir/ftp.conf" <<'EOF'
src optware ftp://packages.example.invalid/feeds/stable
dest root /
EOF
if ipkg_transport_config_is_secure "$test_dir/ftp.conf"; then
    echo "FTP package feed was accepted" >&2
    exit 1
fi
cat > "$test_dir/malformed.conf" <<'EOF'
src optware
dest root /
EOF
if ipkg_transport_config_is_secure "$test_dir/malformed.conf"; then
    echo "malformed source configuration was accepted" >&2
    exit 1
fi
if ipkg_transport_config_is_secure "$test_dir/empty.conf"; then
    echo "missing package configuration was accepted" >&2
    exit 1
fi
echo "ipkg rejects HTTP/FTP/malformed/empty source configuration: PASS"

ln -s "$test_dir/secure.conf" "$test_dir/config-link"
if ipkg_transport_config_is_secure "$test_dir/config-link"; then
    echo "symlink package configuration was accepted" >&2
    exit 1
fi
echo "ipkg refuses symlink package configuration: PASS"

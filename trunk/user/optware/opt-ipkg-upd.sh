#!/bin/sh

self_name="opt-ipkg-upd.sh"
transport_policy="/usr/lib/ipkg_transport_policy.sh"

optware_ipkg_fail()
{
    logger -t "$self_name" "$*"
    exit 1
}

mountpoint -q /opt || optware_ipkg_fail "/opt is not an exact mountpoint; refusing legacy Optware update"
[ -f "$transport_policy" ] && [ ! -L "$transport_policy" ] && [ -r "$transport_policy" ] ||
    optware_ipkg_fail "Secure ipkg transport policy is unavailable"
. "$transport_policy" || optware_ipkg_fail "Cannot load secure ipkg transport policy"

IPKG_CONF_DIR=${IPKG_CONF_DIR:-/etc_ro}
export IPKG_CONF_DIR
if ! ipkg_transport_config_is_secure "$IPKG_CONF_DIR/ipkg.conf"; then
    optware_ipkg_fail "Legacy Optware feed is not HTTPS. The shipped ipkg.nslu2-linux.org source is HTTP-only; configure a trusted HTTPS mirror or disable legacy Optware mode."
fi

export PATH=/opt/sbin:/opt/bin:/usr/sbin:/usr/bin:/sbin:/bin
[ ! -f /opt/bin/opkg ] ||
    optware_ipkg_fail "Entware opkg is present; refusing to mix Entware and legacy Optware package managers"

if [ ! -f /opt/bin/ipkg ]; then
    logger -t "$self_name" "Installing legacy ipkg from verified HTTPS sources"
    mkdir -p /opt/tmp/ipkg ||
        optware_ipkg_fail "Cannot create the ipkg temporary directory"
    /usr/bin/ipkg.sh update ||
        optware_ipkg_fail "HTTPS ipkg source index update failed"
    /usr/bin/ipkg.sh -force-defaults install ipkg-opt ||
        optware_ipkg_fail "HTTPS ipkg-opt package installation failed"
fi

[ -f /opt/bin/ipkg ] && [ ! -L /opt/bin/ipkg ] && [ -x /opt/bin/ipkg ] ||
    optware_ipkg_fail "ipkg-opt did not publish a regular executable /opt/bin/ipkg"

# Use the firmware's transport-hardened shell client. The legacy installed
# binary has no audited HTTPS/certificate-validation contract.
logger -t "$self_name" "Updating legacy ipkg package lists through verified HTTPS"
/usr/bin/ipkg.sh update ||
    optware_ipkg_fail "HTTPS ipkg package index update failed"
logger -t "$self_name" "SUCCESS! Secure legacy Optware update completed"
exit 0

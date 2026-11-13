#!/bin/sh

# This helper is an RC-internal continuation: the caller owns the shared VPN
# lifecycle barrier and has already stopped PPPD with a checked postcondition.
if [ "$#" -ne 1 ] || [ "$1" != "--rc-lifecycle-locked" ]; then
	logger -t "MWAN" "refusing direct launch; use the RC WAN PPP lifecycle path"
	exit 1
fi


sync="$(nvram get pppoesync_enable)"
PPP_NUM="$(nvram get pppoe_num)"
vlanen="$(nvram get vlan_filter)"
vlanid="$(nvram get vlan_vid_cpu)"
userid="$(nvram get wan_pppoe_username)"
userpw="$(nvram get wan_pppoe_passwd)"
usermtu="$(nvram get wan_pppoe_mtu)"
usermru="$(nvram get wan_pppoe_mru)"
if [ "$vlanen" = "1" ]; then
if [ -n "$vlanid" ]; then
ethname="eth3."$vlanid""
fi
else
ethname="eth3"
fi

for i in $(seq 1 $PPP_NUM)
do
        ip link del macvlan"$i" 
        ip link add link "$ethname" macvlan"$i" type macvlan
        ifconfig macvlan"$i" hw ether $(echo $(cat /sys/class/net/"$ethname"/address|awk -F ":" '{print $1":"$2":"$3":"$4":"$5":" }')$(echo "" | awk -F ":" '{printf("%X\n", 16+i);}' i="$i"))
        ifconfig macvlan"$i" up
done
# Existing PPPD sessions were stopped by the RC caller under the VPN
# lifecycle barrier. Never re-kill a saved pidof snapshot here.

for i in $(seq 1 $PPP_NUM)
do
let count=$i-1
if [ "$sync" = "1" ]; then
    echo "syncppp $PPP_NUM user $userid password $userpw +ipv6 refuse-eap plugin rp-pppoe.so nic-macvlan"$i" persist maxfail 0 holdoff 10 ipcp-accept-remote ipcp-accept-local noipdefault mtu "$usermtu" mru "$usermru" usepeerdns default-asyncmap nopcomp noaccomp novj nobsdcomp nodeflate nomppe nomppc lcp-echo-interval 20 lcp-echo-failure 6 minunit 0 linkname wan"$count" ktune" > /tmp/ppp/options.wan0
else
    echo "user $userid password $userpw +ipv6 refuse-eap plugin rp-pppoe.so nic-macvlan"$i" persist maxfail 0 holdoff 10 ipcp-accept-remote ipcp-accept-local noipdefault mtu "$usermtu" mru "$usermru" usepeerdns default-asyncmap nopcomp noaccomp novj nobsdcomp nodeflate nomppe nomppc lcp-echo-interval 20 lcp-echo-failure 6 minunit 0 linkname wan"$count" ktune" > /tmp/ppp/options.wan0
fi
/usr/sbin/pppd file /tmp/ppp/options.wan0
done

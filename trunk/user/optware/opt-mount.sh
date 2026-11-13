#!/bin/sh

self_name="opt-mount.sh"

# check params
[ -z "$1" ] || [ -z "$2" ] && exit 1

mtd_device=`echo "$1" | egrep '^/dev/mtd|^/dev/ubi'`
optware_lifecycle_locked=0

optware_release_lifecycle_lock()
{
	if [ "$optware_lifecycle_locked" = "1" ]; then
		optware_lifecycle_lock_release
		optware_lifecycle_locked=0
	fi
}

if [ -z "$mtd_device" ] ; then
	optw_enable=`nvram get optw_enable`
	[ "$optw_enable" != "1" -a "$optw_enable" != "2" ] && exit 0
else
	# Internal MTD/UBI startup is not covered by the removable-storage caller.
	# Serialize the entire /opt bind/setup/start sequence with opt-start/teardown.
	. /sbin/optware_lifecycle_lock.sh || exit 1
	if ! optware_lifecycle_lock_acquire; then
		logger -t "$self_name" "cannot acquire Optware lifecycle lock; refusing MTD startup"
		exit 1
	fi
	optware_lifecycle_locked=1
fi

# check /opt already mounted
if mountpoint -q /opt; then
	optware_release_lifecycle_lock
	exit 0
fi

# check dir "opt" exist on the drive root
if [ ! -d "$2/opt" ]; then
	optware_release_lifecycle_lock
	exit 0
fi

logger -t "${self_name}" "started [$@]"

# mount /opt (bind only)
mount -o bind "$2/opt" /opt
if [ $? -ne 0 ] ; then
	logger -t "${self_name}" "Mount $2/opt to /opt FAILED! WTF?"
	optware_release_lifecycle_lock
	exit 1
fi

# check dirs exist
for i in "bin" "etc/init.d" "home/admin" "lib" "sbin" "var/log" ; do
	[ ! -d /opt/${i} ] && mkdir -p /opt/${i}
done

# check opt profile exist
if [ ! -f /opt/etc/profile ] ; then
	cat > /opt/etc/profile <<EOF

# If running interactively, then
if [ "\$PS1" ] ; then

    export TERM=xterm
    export LANG=en_US.UTF-8
    export TMP=/opt/tmp
    export TEMP=/opt/tmp

    alias mc='mc -c'

fi;

EOF
fi

# expand home to opt
if [ -d /opt/home/admin ] ; then
	rm -f /home/admin
	ln -sf /opt/home/admin /home/admin
	chmod 700 /opt/home/admin
fi

# prepare /etc/localtime
ln -sf /opt/etc/localtime /etc/localtime

# prepare ssh authorized_keys
if [ -f /etc/storage/authorized_keys ] && [ ! -f /opt/home/admin/.ssh/authorized_keys ] ; then
	mkdir -p /opt/home/admin/.ssh
	cp -f /etc/storage/authorized_keys /opt/home/admin/.ssh
	chmod 700 /opt/home/admin/.ssh
	chmod 600 /opt/home/admin/.ssh/authorized_keys
fi

# check swap file exist
if [ -z "$mtd_device" ] && [ -f /opt/.swap ] ; then
	swap_part=`cat /proc/swaps | grep 'partition' 2>/dev/null`
	swap_file=`cat /proc/swaps | grep 'file' 2>/dev/null`
	if [ -z "$swap_part" ] && [ -z "$swap_file" ] ; then
		swapon /opt/.swap
		[ $? -eq 0 ] && logger -t "${self_name}" "Activate swap file /opt/.swap SUCCESS!"
	fi
fi

# create system tweak script
system_init_d="/opt/etc/init.d/S01system"
if [ ! -f "$system_init_d" ]  ; then
	cat > "$system_init_d" <<EOF
#!/bin/sh

### Custom user script for system tweak

func_start()
{
	echo "Start system tweak"
	# insert your custom code below
}

func_stop()
{
	echo "Stop system tweak"
	# insert your custom code below
}

case "\$1" in
start)
	func_start
	;;
stop)
	func_stop
	;;
restart)
	func_stop
	func_start
	;;
*)
	echo "Usage: \$0 {start|stop|restart}"
	exit 1
	;;
esac

EOF
	chmod 755 "$system_init_d"
fi

# create iptables update script
iptables_script="/opt/bin/update_iptables.sh"
iptables_init_d="/opt/etc/init.d/S10iptables"
if [ ! -f "$iptables_script" ]  ; then
	cat > "$iptables_script" <<EOF
#!/bin/sh

### Custom user script for post-update iptables
### This script auto called after internal firewall restart
### First param is:
###  "start" (call at start optware),
###  "stop" (call before stop optware),
###  "update" (call after internal firewall restart).
### Include you custom rules for iptables below:

case "\$1" in
start|update)
	# add iptables custom rules
	echo "firewall started"
	;;
stop)
	# delete iptables custom rules
	echo "firewall stopped"
	;;
*)
	echo "Usage: \$0 {start|stop|update}"
	exit 1
	;;
esac

EOF
	chmod 755 "$iptables_script"
fi

if [ ! -L "$iptables_init_d" ]  ; then
	ln -sf "$iptables_script" "$iptables_init_d"
fi

# create script for WPS button events handling
wps_script="/opt/bin/on_wps.sh"
if [ ! -f "$wps_script" ]  ; then
	cat > "$wps_script" <<EOF
#!/bin/sh

### Custom user script for WPS button events handling

case "\$1" in
1)
	# WPS short pressed
	echo "WPS button short pressed!"
	;;
2)
	# WPS long pressed
	echo "WPS button long pressed!"
	;;
esac

EOF
	chmod 755 "$wps_script"
fi

# create script for printer hotplug event handling
lph_script="/opt/bin/on_hotplug_printer.sh"
if [ ! -f "$lph_script" ]  ; then
	cat > "$lph_script" <<EOF
#!/bin/sh

[ -z "\$1" ] && exit 1

### Custom user script for printer hotplug event handling
### First param is /dev/usb/lp[0-9]

### Example: load firmware to printer HP LJ1020
lpfw="/opt/share/firmware/sihp1020.dl"
if [ -r "\$lpfw" ] ; then
	cat "\$lpfw" > "\$1"
fi

EOF
	chmod 755 "$lph_script"
fi

# mark opt needed start
if [ -z "$mtd_device" ] ; then
	nvram settmp usb_opt_start=1
	exit 0
fi

logger -t "${self_name}" "call /opt/etc/init.d"

# extend path to /opt
export PATH=/opt/sbin:/opt/bin:/usr/sbin:/usr/bin:/sbin:/bin

# Use the same supervised, rollback-capable startup contract as opt-start.sh.
if ! optware_lifecycle_start_services /opt/etc/init.d; then
	logger -t "${self_name}" "MTD Optware service startup failed; see hook and rollback errors"
	optware_release_lifecycle_lock
	exit 1
fi

optware_release_lifecycle_lock
exit 0

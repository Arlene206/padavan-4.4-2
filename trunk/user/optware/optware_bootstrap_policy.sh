#!/bin/sh
# Harden the upstream Entware installer before executing it as root.

optware_bootstrap_patch_installer()
{
    [ "$#" -eq 2 ] || return 2
    obp_source=$1
    obp_destination=$2
    [ -f "$obp_source" ] && [ ! -L "$obp_source" ] && [ -r "$obp_source" ] || return 1
    [ -n "$obp_destination" ] && [ ! -e "$obp_destination" ] && [ ! -L "$obp_destination" ] || return 1

    if ! awk '
        NR == 1 {
            if ($0 != "#!/bin/sh") invalid = 1
            print
            print "set -e"
            next
        }
        {
            line = $0
            gsub(/http:\/\/bin[.]entware[.]net\//, "https://bin.entware.net/", line)
            if (index(line, "http://") != 0) insecure_url = 1
            if (line ~ /^[[:space:]]*URL[[:space:]]*=[[:space:]]*https:\/\/bin[.]entware[.]net\/.*$/) url_count++
            if (line ~ /^[[:space:]]*wget[[:space:]]/) {
                download_count++
                is_config_download = (line ~ /^[[:space:]]*wget[[:space:]]+\$URL\/opkg[.]conf[[:space:]]+-O[[:space:]]+\/opt\/etc\/opkg[.]conf[[:space:]]*$/)
                sub(/wget[[:space:]]+/, "wget -T 60 ", line)
                print line
                if (is_config_download) {
                    config_download_count++
                    print "if ! sed -i \"s|http://bin.entware.net/|https://bin.entware.net/|g\" /opt/etc/opkg.conf; then"
                    print "    exit 1"
                    print "fi"
                    print "if grep -q \"http://\" /opt/etc/opkg.conf; then"
                    print "    exit 1"
                    print "fi"
                }
                next
            }
            print line
        }
        END {
            if (NR == 0 || invalid || url_count != 1 || download_count != 2 ||
                config_download_count != 1 || insecure_url) exit 1
        }
    ' "$obp_source" > "$obp_destination"; then
        rm -f "$obp_destination"
        return 1
    fi

    if [ -L "$obp_destination" ] || [ ! -f "$obp_destination" ] ||
       [ ! -s "$obp_destination" ] || ! sh -n "$obp_destination"; then
        rm -f "$obp_destination"
        return 1
    fi
    return 0
}

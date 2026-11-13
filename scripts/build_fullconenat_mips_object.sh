#!/bin/sh
# Compile the production FULLCONENAT translation unit using the XY-C1
# board configuration and the project's prebuilt MIPS32r2 toolchain.
# This is an object-only check, not a complete firmware build.
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
project_dir="$repo_root/trunk"
kernel_dir="$project_dir/linux-4.4.x"
toolchain_root=${MIPS_TOOLCHAIN_ROOT:-"$repo_root/toolchain/mipsel-linux-musl"}
cross_prefix="$toolchain_root/bin/mipsel-linux-musl-"
jobs=${JOBS:-2}

if [ ! -x "${cross_prefix}gcc" ]; then
  echo "missing MIPS compiler: ${cross_prefix}gcc" >&2
  exit 1
fi
command -v readelf >/dev/null 2>&1 || {
  echo "readelf is required to verify the resulting target object" >&2
  exit 1
}

# Reproduce the normal XY-C1 product configuration path. In particular,
# trunk/configure must derive conntrack-event support from FULLCONENAT even
# when the product template disables optional SQM.
cp "$project_dir/configs/templates/XY-C1.config" "$project_dir/.config"
{
  printf '\nCONFIG_CROSS_COMPILE=mipsel-linux-musl\n'
  printf 'CONFIG_CROSS_COMPILER_ROOT=%s\n' "$toolchain_root"
  printf 'CONFIG_CCACHE=n\n'
} >> "$project_dir/.config"

(
  cd "$project_dir"
  bash -n ./configure
  ./configure
)

(
  cd "$kernel_dir"
  make ARCH=mips CROSS_COMPILE="$cross_prefix" olddefconfig

  # These are the real XY-C1 board selections, not generic defconfig defaults.
  grep -qx 'CONFIG_RALINK=y' .config
  grep -qx 'CONFIG_RALINK_MT7621=y' .config
  grep -qx 'CONFIG_SOC_MT7621=y' .config
  grep -qx 'CONFIG_CPU_LITTLE_ENDIAN=y' .config
  ! grep -qx 'CONFIG_CPU_BIG_ENDIAN=y' .config
  grep -qx 'CONFIG_CPU_MIPS32_R2=y' .config

  # FULLCONENAT's Kconfig dependency must remain enabled when SQM is off.
  grep -qx 'CONFIG_NF_CONNTRACK_EVENTS=y' .config
  grep -qx 'CONFIG_NF_NAT_IPV6=m' .config
  grep -qx 'CONFIG_NETFILTER_XT_TARGET_FULLCONENAT=y' .config

  make -j"$jobs" ARCH=mips CROSS_COMPILE="$cross_prefix" \
    net/netfilter/xt_FULLCONENAT.o
)

object="$kernel_dir/net/netfilter/xt_FULLCONENAT.o"
[ -s "$object" ]
header=$(readelf -h "$object")
printf '%s\n' "$header" | grep -Eq 'Class:[[:space:]]+ELF32'
printf '%s\n' "$header" | grep -Eq 'Data:[[:space:]]+2.s complement, little endian'
printf '%s\n' "$header" | grep -Eq 'Machine:[[:space:]]+MIPS'
echo "FULLCONENAT XY-C1 MIPS32r2 IPv4/IPv6 object build: PASS"
readelf -h "$object" | grep -E 'Class:|Data:|Machine:'

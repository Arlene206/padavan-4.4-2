#!/bin/sh
# Compile the production FULLCONENAT translation unit with the project's
# prebuilt MIPS32r2 toolchain. This is an object-only check, not a firmware build.
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
kernel_dir="$repo_root/trunk/linux-4.4.x"
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

(
  cd "$kernel_dir"
  make ARCH=mips CROSS_COMPILE="$cross_prefix" mt7621_defconfig
  bash scripts/config --file .config \
    --enable RALINK \
    --enable SOC_MT7621 \
    --enable CPU_LITTLE_ENDIAN \
    --enable CPU_MIPS32_R2 \
    --enable NETFILTER_ADVANCED \
    --enable IPV6 \
    --enable NF_CONNTRACK_EVENTS \
    --module NF_NAT_IPV6 \
    --module NETFILTER_XT_TARGET_FULLCONENAT
  make ARCH=mips CROSS_COMPILE="$cross_prefix" olddefconfig

  # mt7621_defconfig omits its Ralink SoC and byte-order choices. Pin them
  # explicitly so arch/mips does not add -EB beside the project's -EL.
  grep -qx 'CONFIG_RALINK=y' .config
  grep -qx 'CONFIG_SOC_MT7621=y' .config
  grep -qx 'CONFIG_CPU_LITTLE_ENDIAN=y' .config
  grep -qx 'CONFIG_CPU_MIPS32_R2=y' .config
  ! grep -qx 'CONFIG_CPU_BIG_ENDIAN=y' .config
  grep -qx 'CONFIG_NF_CONNTRACK_EVENTS=y' .config
  grep -qx 'CONFIG_NF_NAT_IPV6=m' .config
  grep -qx 'CONFIG_NETFILTER_XT_TARGET_FULLCONENAT=m' .config

  make -j"$jobs" ARCH=mips CROSS_COMPILE="$cross_prefix" \
    net/netfilter/xt_FULLCONENAT.o
)

object="$kernel_dir/net/netfilter/xt_FULLCONENAT.o"
[ -s "$object" ]
header=$(readelf -h "$object")
printf '%s\n' "$header" | grep -Eq 'Class:[[:space:]]+ELF32'
printf '%s\n' "$header" | grep -Eq 'Data:[[:space:]]+2.s complement, little endian'
printf '%s\n' "$header" | grep -Eq 'Machine:[[:space:]]+MIPS'
echo "FULLCONENAT MIPS32r2 IPv4/IPv6 object build: PASS"
readelf -h "$object" | grep -E 'Class:|Data:|Machine:'

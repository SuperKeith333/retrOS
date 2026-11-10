#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ISO="$ROOT_DIR/build-uefi/retrOS.iso"

if [[ ! -f "$ISO" ]]; then
	echo "ISO not found. Build it first."
	exit 1
fi

qemu-system-x86_64 \
	-machine q35 \
	-m 512M \
	-bios /usr/share/edk2/x64/OVMF.4m.fd \
	-cdrom "$ISO" \
	-boot d \
	-serial stdio \
	-drive format=raw,file=build-uefi/esp.img \
	-net none
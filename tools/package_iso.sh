
#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build-uefi"
ISO_DIR="$BUILD_DIR/iso"
ESP_IMAGE="$ISO_DIR/esp.img"
OUTPUT_ISO="$BUILD_DIR/retrOS.iso"

BOOTLOADER="$BUILD_DIR/bootloader/BOOTX64.EFI"
KERNEL="$BUILD_DIR/kernel/kernel.elf"

# Verify build outputs exist
test -f "$BOOTLOADER"
test -f "$KERNEL"

mkdir -p "$ISO_DIR/EFI/BOOT" "$ISO_DIR/boot"

# Copy files into ISO directory
cp "$BOOTLOADER" "$ISO_DIR/EFI/BOOT/BOOTX64.EFI"
cp "$KERNEL" "$ISO_DIR/boot/kernel.elf"

# Create a 1GB FAT32 EFI System Partition
dd if=/dev/zero of="$ESP_IMAGE" bs=1M count=1024 status=none
mkfs.fat -F 32 "$ESP_IMAGE"

# Populate the EFI partition
mmd -i "$ESP_IMAGE" ::/EFI
mmd -i "$ESP_IMAGE" ::/EFI/BOOT
mcopy -i "$ESP_IMAGE" "$BOOTLOADER" ::/EFI/BOOT/BOOTX64.EFI
mmd   -i "$ESP_IMAGE" ::/boot
mcopy -i "$ESP_IMAGE" "$KERNEL" ::/boot/kernel.elf

# Build UEFI-bootable ISO
xorriso -as mkisofs \
	-iso-level 3 \
	-full-iso9660-filenames \
	-volid "RETROS" \
	-eltorito-alt-boot \
	-e esp.img \
	-no-emul-boot \
	-o "$OUTPUT_ISO" \
	"$ISO_DIR"

echo "Created: $OUTPUT_ISO"

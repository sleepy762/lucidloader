#!/bin/sh
# Check if the directory exists and create it if it doesn't
[ -d iso ] || mkdir iso

# Create a FAT type image and add the EFI file into it
dd if=/dev/zero of=fat.img bs=1k count=1440
mformat -i fat.img -f 1440 ::
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mcopy -i fat.img bootx64.efi ::/EFI/BOOT

# Create an ISO image with our EFI file and start uefi qemu to test it
cp fat.img iso
xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o cdimage.iso iso
qemu-system-x86_64 -L ovmf/ -pflash ovmf/OVMF.fd -cdrom cdimage.iso

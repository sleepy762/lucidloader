#!/bin/bash
# This script is used for quick testing

[ -d add-to-image ] || mkdir add-to-image
[ -d iso ] || mkdir iso

# Create a FAT type image
dd if=/dev/zero of=fat.img bs=1k count=34400
mformat -i fat.img -s 64 -t 225 -h 16 ::
# Create the necessary directories
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mmd -i fat.img ::/EFI/lucidloader
# Copy the files
cp lucidloader_x64.efi bootx64.efi # This is done in order to start the boot manager automatically
mcopy -i fat.img bootx64.efi ::/EFI/BOOT
mcopy -i fat.img add-to-image/* ::/EFI/lucidloader

# Create an ISO image with our EFI file and start uefi qemu to test it
cp fat.img iso
xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o cdimage.iso iso
rm -rf iso
qemu-system-x86_64 -cpu qemu64 -bios ovmf/OVMF.fd -drive file=cdimage.iso,if=ide -net none -enable-kvm

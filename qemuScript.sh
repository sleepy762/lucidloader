#!/bin/bash
# This script is used for quick testing

make -j8

[ -d binaries ] || mkdir binaries
[ -d iso ] || mkdir iso

# Create a FAT type image and add the EFI file into it
dd if=/dev/zero of=fat.img bs=1k count=14400
mformat -i fat.img -s 64 -t 225 -h 16 ::
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mmd -i fat.img ::/EFI/apps
mcopy -i fat.img bootx64.efi ::/EFI/BOOT
mcopy -i fat.img binaries/* ::/EFI/apps

# Create an ISO image with our EFI file and start uefi qemu to test it
cp fat.img iso
xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o cdimage.iso iso
rm -rf iso
qemu-system-x86_64 -cpu qemu64 -bios ovmf/OVMF.fd -drive file=cdimage.iso,if=ide -net none -enable-kvm

#!/bin/sh
./cleanScript.sh
make

[ -d binaries ] || mkdir binaries
[ -d iso ] || mkdir iso

# Create a FAT type image and add the EFI file into it
dd if=/dev/zero of=fat.img bs=1k count=14400
mformat -i fat.img -s 64 -t 225 ::
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mmd -i fat.img ::/EFI/test
mmd -i fat.img ::/EFI/Microsoft
mmd -i fat.img ::/EFI/Microsoft/Boot
mcopy -i fat.img bootx64.efi ::/EFI/BOOT
mcopy -i fat.img binaries/* ::/EFI/test
mcopy -i fat.img binaries/bootmgfw.efi ::/EFI/Microsoft/Boot

# Create an ISO image with our EFI file and start uefi qemu to test it
cp fat.img iso
xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o cdimage.iso iso
qemu-system-x86_64 -L ovmf/ -pflash ovmf/OVMF.fd -cdrom cdimage.iso

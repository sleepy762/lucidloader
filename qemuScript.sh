dd if=/dev/zero of=fat.img bs=1k count=1440
mformat -i fat.img -f 1440 ::
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mcopy -i fat.img bootx64.efi ::/EFI/BOOT

cp fat.img iso
xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o cdimage.iso iso
qemu-system-x86_64 -L ovmf/ -pflash ovmf/OVMF.fd -cdrom cdimage.iso

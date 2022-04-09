## Emulating for development

Emulation is used to test code changes quickly.

#### Additional Dependencies

Arch GNU/Linux (pacman): `mtools` `qemu` `ovmf` `xorriso`

Debian GNU/Linux (apt): `mtools` `qemu-system-x86` `ovmf` `xorriso`

Gentoo GNU/Linux (emerge): `sys-fs/mtools` `app-emulation/qemu` `sys-firmware/edk2-ovmf` `dev-libs/libisoburn`
#
Copy `OVMF.fd` in the OVMF directory installed on your pc into a directory with the name `ovmf`.

The OVMF files can be found in these paths:
- in Arch: `/usr/share/ovmf/x64/OVMF.fd` 
- in Debian: `/usr/share/ovmf/OVMF.fd`
- in Gentoo: `/usr/share/edk2-ovmf/OVMF_CODE.fd` (make sure to rename it to `OVMF.fd` in the `ovmf` directory)

Running `./launch_qemu.sh` will compile the sources, create a FAT image and convert it into ISO, and start QEMU with the bootloader.

Any files that you want to have in the FAT image should be in the `add-to-image` directory, and they will be stored in the path `\EFI\lucidloader` on the fat image.

After playing around with the bootloader in QEMU, if you want to check the filesystem on the FAT image, it's possible to mount the image onto a directory using `mount_image.sh`.
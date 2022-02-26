# Bootloader

A UEFI bootloader that aims to be a simpler implementation of GRUB.

This bootloader is being developed using the [POSIX-UEFI](https://gitlab.com/bztsrc/posix-uefi) wrapper library.


## Dependencies

Arch GNU/Linux (pacman): `make` `clang` `lld` `efibootmgr`

Debian GNU/Linux (apt): `make` `clang` `lld` `efibootmgr`

Gentoo GNU/Linux (emerge): `sys-devel/make` `sys-devel/clang` `sys-devel/lld` `sys-boot/efibootmgr`

The Makefile will prefer clang+lld over gcc+ld, but if clang isn't installed then gcc will be used.


## Emulating

Emulation is used to test code changes quickly.

#### Additional Dependencies

Arch GNU/Linux (pacman): `mtools` `qemu` `ovmf` `xorriso`

Debian GNU/Linux (apt): `mtools` `qemu-system-x86` `ovmf` `xorriso`

Gentoo GNU/Linux (emerge): `sys-fs/mtools` `app-emulation/qemu` `sys-firmware/edk2-ovmf` `dev-libs/libisoburn`

Copy `OVMF.fd` in the OVMF directory installed on your pc into a directory with the name `ovmf`.

The OVMF files can be found in these paths:
- in Arch: `/usr/share/ovmf/x64/OVMF.fd` 
- in Debian: `/usr/share/ovmf/OVMF.fd`
- in Gentoo: `/usr/share/edk2-ovmf/OVMF_CODE.fd` (make sure to rename it to `OVMF.fd` in the `ovmf` directory)

Running `./launch_qemu.sh` will compile the sources, create a FAT image and convert it into ISO, and start QEMU with the bootloader.

Any files that you want to have in the FAT image should be in the `add-to-image` directory, and they will be stored in the path `\EFI\ezboot` on the fat image.

After playing around with the bootloader in QEMU, if you want to check the filesystem on the FAT image, it's possible to mount the image onto a directory using `mount_image.sh`.


## Building

Running `make` in the root directory will create the bootloader `.efi` file.

Clang is the preferred compiler for compiling the code and it will be used if clang is installed, but gcc can also be used. 

If you have clang installed and want to compile with gcc, add `USE_GCC=1` into the Makefile in the root directory (before the include line). If clang is not installed, the code will be compiled with gcc automatically.

## Installing

To install the bootloader, simply run the installer script (installer.sh) as root. A new boot entry will be created with efibootmgr and the bootloader will be run the next time the device is started.

To update the bootloader, it is as simple as running the installer script again.

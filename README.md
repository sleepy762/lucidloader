# Bootloader

This bootloader is being developed using the [POSIX-UEFI](https://gitlab.com/bztsrc/posix-uefi) wrapper library.

#### Dependencies
Arch Linux Dependencies (pacman): `make`(part of `base-devel`), `clang`, `mtools`, `qemu`, `ovmf`, `xorriso`, `lld`

Debian Dependencies (apt): `make`(part of `build-essential`), `clang`, `mtools`, `qemu-system-x86`, `ovmf`, `xorriso`, `lld`

Gentoo Dependencies (emerge): `sys-devel/make`, `sys-devel/clang`, `sys-fs/mtools`, `app-emulation/qemu`, `sys-firmware/edk2-ovmf`, `dev-libs/libisoburn`, `sys-devel/lld`


## Building
Copy `OVMF.fd` in the OVMF directory installed on your pc into a directory with the name `ovmf`.

The OVMF files can be found in these paths:
- in Arch: `/usr/share/ovmf/x64/OVMF.fd` 
- in Debian: `/usr/share/ovmf/OVMF.fd`
- in Gentoo: `/usr/share/edk2-ovmf/OVMF_CODE.fd` (make sure to rename it to `OVMF.fd` in the `ovmf` directory)

**DO NOT** use gcc to compile the project because it will not work.

Running `make` in the root directory will create the bootloader `.efi` file.

Alternatively, running `./qemuScript.sh` will compile the sources (runs `make -j8` by default), create a FAT image and convert it into ISO, and start QEMU with the bootloader.
Any files that you want to have in the FAT image should be in the `binaries` folder, and they will be stored in the path `\EFI\apps\` on the fat image.

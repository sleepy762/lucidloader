# Bootloader

A UEFI bootloader that aims to be a simpler implementation of GRUB.

This bootloader is being developed using the [POSIX-UEFI](https://gitlab.com/bztsrc/posix-uefi) wrapper library.


#### Dependencies

Arch GNU/Linux (pacman): `make` (part of `base-devel`), `clang` / `gcc`, `mtools`, `qemu`\*, `ovmf`\*, `xorriso`, `lld`, `efibootmgr`

Debian GNU/Linux (apt): `make` (part of `build-essential`), `clang` / `gcc`, `mtools`, `qemu-system-x86`\*, `ovmf`\*, `xorriso`, `lld`, `efibootmgr`

Gentoo GNU/Linux (emerge): `sys-devel/make`, `sys-devel/clang` / `sys-devel/gcc`, `sys-fs/mtools`, `app-emulation/qemu`\*, `sys-firmware/edk2-ovmf`\*, `dev-libs/libisoburn`, `sys-devel/lld`, `sys-boot/efibootmgr`

\* - Optional (used for emulation in QEMU), not required for building and installing.

The Makefile will prefer clang over gcc, but if clang isn't installed then gcc will be used.


## Emulating

Installing your distro's OVMF package and QEMU is required for this part.

Copy `OVMF.fd` in the OVMF directory installed on your pc into a directory with the name `ovmf`.

The OVMF files can be found in these paths:
- in Arch: `/usr/share/ovmf/x64/OVMF.fd` 
- in Debian: `/usr/share/ovmf/OVMF.fd`
- in Gentoo: `/usr/share/edk2-ovmf/OVMF_CODE.fd` (make sure to rename it to `OVMF.fd` in the `ovmf` directory)

Running `./qemuScript.sh` will compile the sources (runs `make -j8` by default), create a FAT image and convert it into ISO, and start QEMU with the bootloader.

Any files that you want to have in the FAT image should be in the `binaries` folder, and they will be stored in the path `\EFI\apps\` on the fat image.


## Building

Running `make` in the root directory will create the bootloader `.efi` file.

Clang is the preferred compiler for compiling the code and it will be used if clang is installed, but gcc can also be used. 

If you have clang installed and want to compile with gcc, add `USE_GCC=1` into the Makefile in the root directory (before the include line). If clang is not installed, the code will be compiled with gcc automatically.

# Bootloader

Arch Linux Dependencies: `base-devel`, `clang`, `mtools`, `qemu`, `ovmf`, `xorriso`, `lld`
Debian Dependencies: `build-essential`, `clang`, `mtools`, `qemu-system-x86`, `ovmf`, `xorriso`, `lld`

Copy `OVMF.fd` in the OVMF directory installed on your pc into a directory with the name `ovmf`.
The OVMF files can be found in these paths:
- in Arch: `/usr/share/ovmf/x64/OVMF.fd` 
- in Debian: `/usr/share/ovmf/OVMF.fd`

Run `./qemuScript.sh`, it will compile the sources (runs `make -j8` by default), create a FAT image and convert it into ISO, and start QEMU with the image.
Any binary files that you want to have in the FAT image should be in the `binaries` folder.

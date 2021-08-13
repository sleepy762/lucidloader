# Bootloader

Dependencies: `base-devel`, `clang`, `mtools`, `qemu`, `ovmf`, `xorriso`, `lld`

Copy the files in the OVMF directory installed on your pc (usually `/usr/share/ovmf/x64/`) into a directory with the name `ovmf`.

Run `./qemuScript.sh`, it will compile the sources, create a FAT image, and start QEMU with the image
Any binary files that you want to have in the FAT image should be in the `binaries` folder

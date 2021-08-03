# Bootloader

Dependencies: `base-devel`, `clang`, `mtools`, `qemu`, `ovmf`, `xorriso`, `lld`

Copy the files in the OVMF directory installed on your pc (usually `/usr/share/ovmf/x64/`) into a directory with the name `ovmf`. Then follow the instructions below.

After every code update:
1. Run `make` (Make sure it compiles without errors)
2. Run `./qemuScript.sh` in order to add the UEFI application into an ISO file
3. QEMU will run with the application

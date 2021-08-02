# boot loader

Dependencies: `base-devel`, `clang`, `mtools`, `qemu`, `ovmf`, `xorriso`

After every code update:
1. Run `make` (Make sure it compiles without errors)
2. Run `./qemuScript.sh` in order to add the UEFI application into an ISO file
3. QEMU will run with the application

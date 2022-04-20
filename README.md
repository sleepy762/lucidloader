# LucidLoader

An interactive UEFI boot manager that aims to be light and easy to use.

This boot manager is being developed using the [POSIX-UEFI](https://gitlab.com/bztsrc/posix-uefi) wrapper library.

## Features
- Very lightweight.
- Simple and easy to use configuration.
- Shell environment with useful commands.
- Text editor.
- Able to boot operating systems and UEFI apps.
- Can automatically detect a Linux kernel along with its version.

## Dependencies

Arch (pacman): `make` `clang` `lld` `efibootmgr`

Debian (apt): `make` `clang` `lld` `efibootmgr`

Gentoo (emerge): `sys-devel/make` `sys-devel/clang` `sys-devel/lld` `sys-boot/efibootmgr`

## Building

Run `make` in the root directory of the project to create the boot manager's `.efi` file.

**The boot manager is unsigned at the moment and you will have to disable secure boot in your bios or sign it yourself.**

The boot manager fully supports the `x86_64` architecture. Compiling for `aarch64` is possible, but untested. It is possible to change the architecture to `aarch64` by editing the Makefile in the root directory of the boot manager source code and changing the `ARCH` variable. 
#
Clang is the preferred compiler for compiling the code and it will be used if clang is installed, but gcc can also be used.

If you have clang installed and want to compile with gcc, add `USE_GCC=1` into the Makefile in the root directory (before the include line). If clang is not installed, the code will be compiled with gcc automatically.

**Compiling this project with gcc+ld is not supported and it may not work, do that at your own risk.**

See [EMULATING.md](EMULATING.md) for instructions on how to run the boot manager in QEMU to test all kinds of code changes quickly and easily.

## Installing

To install the boot manager, simply run the installer script (installer.sh) as root. A new boot entry will be created with efibootmgr and the boot manager will be run the next time the device is started.

To update the boot manager, it is as simple as running the installer script again. The configuration file will stay unchanged.

See [CONFIGURING.md](CONFIGURING.md) to see how to write the config file.

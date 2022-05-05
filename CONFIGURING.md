# Configuring the boot manager

## Bootable entries

The configuration file is named `config.cfg` and it is saved in the directory of the boot manager `ESP/EFI/lucidloader` (ESP is the EFI System Partition, which is usually mounted as `/boot` or `/boot/efi`). 

Every entry in the config file will be a section of text that must contain the `name` and `path` keys OR `name` and `kerneldir` keys. Each entry is separated by an empty line. In addition, the first entry is considered as the default entry.

The path specified in `path` and `kerneldir` begins at the mount point of the EFI System Partition(ESP). For instance, if it is mounted at `/boot`, then `/boot` is the root directory for the boot manager. You can find out where the ESP is mounted using `lsblk`. It is also worth noting that the delimiter between directories in UEFI is `\` and not `/`. Spaces are also allowed and don't need to be escaped. So an example path will look like this: `\EFI\Arch Linux\vmlinuz-linux`. 

Lines that start with `#` are treated as comments and will be ignored by the config parser.

Available keys:
- `name` - The name of the entry which will be shown in the boot menu.
- `path` - The absolute path to the binary which the boot manager is going to load. **Incompatible with `kerneldir`.**
- `kerneldir` - The absolute path to a directory with a (Linux) kernel (whose name begins with `vmlinuz`). The boot manager will automatically detect the kernel file and the kernel version. It will also replace the characters `%v` in the args with the kernel version string. As a result, the user won't have to edit the config with every kernel update. Highly recommended for Linux systems whose kernel file name can change. Make sure there is only ONE kernel in the specified directory. **Incompatible with `path`.**
- `args` - (Optional) Arguments which will be passed to the binary. When `kerneldir` is defined and the boot manager detects the kernel version, it will substitute the characters `%v` with the kernel version string. It's possible to have multiple lines with this key, they will be concatenated into the full arguments string in order.
- `initrd` - (Optional) The absolute path to an initrd file(initramfs file and/or microcode file) which is used when booting Linux kernels. The boot manager will substitute the characters `%v` with the kernel version string here too. It's possible to have multiple lines with this key, they will be concatenated into the full arguments string in order. If a microcode is present, make sure its loaded before the initramfs.
- `protocol` - (Optional) Sets the boot protocol for the entry. Available boot protocols: `efilaunch`(default), `linux`.

Writing key and value pairs is in the following format: `key:value`. The config is flexible with spaces and you can add as many spaces as you want before and after the delimiter, the key, and the value. Only leading and trailing spaces will be trimmed.

The following is an example of a configuration file:

```
# Runtime settings (see below)
timeout: 5

# Example of a basic entry
name:   Arch Linux
path:   EFI\Arch\vmlinuz-linux
initrd: EFI\Arch\intel-ucode.img
initrd: EFI\Arch\initramfs-linux.img
args:   root=UUID=4ec51638-9069-4a28-9b85-6f2352991ee5 rw loglevel=3
# efilaunch protocol is implicitly used here since it's the default

# Example usage of kerneldir
# the kernel name is 'vmlinuz-5.15.7-gentoo', and the initramfs is 'initramfs-5.15.7-gentoo.img'
name:       Gentoo
kerneldir:  EFI\Gentoo
initrd:     EFI\Gentoo\intel-uc.img
initrd:     EFI\Gentoo\initramfs-%v-gentoo.img
args:       root=UUID=cf2aba83-e914-4613-89fd-9667bb734779 rw loglevel=3
# Will work with efilaunch protocol, if the kernel was compiled with EFISTUB
protocol:   linux

# This path is the same on every system with Windows 10, so you can copy this path and use it to boot Windows
name: Windows
path: EFI\Microsoft\Boot\bootmgfw.efi

# The boot manager can also load any UEFI app and not just operating systems
name: Some UEFI App
path: \path\to\UEFI\app
args: arg1
args: arg2
args: arg3 arg4 arg5....
```

There are 4 entries in this configuration, each entry is separated by an empty line, and each entry has the keys `name` and `path`, or `name` and `kerneldir`.

## Global runtime configuration

These are special keys that you can put anywhere in the config file and they will change runtime configuration in the boot manager.

Available keys:
- `timeout` - Controls the amount of time the boot manager waits before automatically booting the FIRST entry, if no keys are pressed during the count down. Setting the value to `0` will boot the first entry immediately. Setting the value to `-1` will disable the timeout.

## Linux Kernel Args

This is not a comprehensive explanation about Linux kernel parameters, it's here to help configure the boot manager to be able to boot a Linux kernel.

In order to boot a Linux kernel, there are some arguments that it must get. Note that each kernel argument must be separated with a space. Refer to the example config above to see how to pass args to the kernel with the boot manager.

First, we have to specify the UUID of the root filesystem. You can find what partition the root directory is mounted at with `lsblk`, and then get the UUID of that partition with `blkid`. The argument format is `root=UUID=uuid here`, for example:

```root=UUID=4ec51638-9069-4a28-9b85-6f2352991ee5```

Second, if you have an initramfs file, it should be specified with the `initrd` key, or added with `args`. The argument format is `initrd=path to initramfs file` (if using `args`). The path must have backslashes, and not forward slashes.

To load a microcode, add another `initrd` key before the initramfs one, and pass a path to the microcode file. **Make sure the microcode initrd line is placed before the initramfs initrd line, the order matters.**

It's also recommended to add the arguments `rw` and `loglevel=3`.

Read more about Linux kernel parameters [here](https://www.kernel.org/doc/html/latest/admin-guide/kernel-parameters.html).

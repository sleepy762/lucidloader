# Configuring the bootloader

## Bootable entries

The configuration file is named `ezboot-config.cfg` and it is saved in the directory of the bootloader `ESP/EFI/ezboot` (ESP is the EFI System Partition, which is usually mounted as `/boot` or `/boot/efi`). 

Every entry in the config file will be a section of text that must contain the `name` and `path` keys OR `name` and `kerneldir` keys. Each entry is separated by an empty line. In addition, the first entry is considered as the default entry.

The path specified in `path`, begins at the mount point of the EFI System Partition(ESP). For instance, if it is mounted at `/boot`, then `/boot` is the root directory for the bootloader. You can find out where the ESP is mounted using `lsblk`. It is also worth noting that the delimiter between directories in UEFI is `\` and not `/`. Spaces are also allowed and don't need to be escaped. So an example path will look like this: `\EFI\Arch Linux\vmlinuz-linux`. 

Lines that start with `#` are treated as comments and will be ignored by the config parser.

Available keys:
- `name` - The name of the entry which will be shown in the boot menu.
- `path` - The absolute path to the binary which the bootloader is going to load. **Incompatible with `kerneldir`.**
- `kerneldir` - The absolute path to a directory with a (Linux) kernel (whose name begins with `vmlinuz`). The bootloader will automatically detect the kernel file and the kernel version. It will also replace the characters `%v` in the args with the kernel version string. As a result, the user won't have to edit the config with every kernel update. Highly recommended for Linux systems whose kernel file name can change. Make sure there is only ONE kernel in the specified directory. **Incompatible with `path`.**
- `args` - (Optional) Arguments which will be passed to the binary. When `kerneldir` is defined and the bootloader detects the kernel version, it will substitute the characters `%v` with the kernel version string.

Writing key and value pairs is in the following format: `key=value`. Note that there are NO SPACES to the right and to the left of the `=` character. 

Let's see an example.

```
# Runtime settings (see below)
timeout = 5

# Example of a basic entry
name=Arch Linux
path=EFI\Arch\vmlinuz-linux
args=root=UUID=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx initrd=EFI\Arch\initramfs-linux.img rw loglevel=3

# Example usage of kerneldir
# the kernel name is 'vmlinuz-5.15.7-gentoo', and the initramfs is 'initramfs-5.15.7-gentoo.img'
name=Gentoo
kerneldir=EFI\Gentoo
args=root=UUID=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx initrd=EFI\Gentoo\initramfs-%v-gentoo.img rw loglevel=3

name=Windows
path=EFI\Boot\Microsoft\bootmgfw.efi

# The bootloader can also load any UEFI app and not just operating systems
name=Some UEFI App
path=\path\to\UEFI\app
args=args to pass to the app
```

Here we can see 4 entries, each entry is separated by an empty line, and each has the keys `name` and `path`, or `name` and `kerneldir`.

## Global runtime configuration

These are special keys that you can put anywhere in the config file and they will change runtime configuration in the bootloader.

Available keys:
- `timeout` - Controls the amount of time the bootloader waits before automatically booting the FIRST entry, if no keys are pressed during the count down. Setting the value to `0` will boot the first entry immediately. Setting the value to `-1` will disable the timeout.

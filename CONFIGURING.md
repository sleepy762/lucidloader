# Configuring the bootloader

## Bootable entries

The configuration file is named `ezboot-config.cfg` and it is saved in the directory of the bootloader `ESP/EFI/ezboot` (ESP is the EFI System Partition, which is usually mounted as `/boot` or `/boot/efi`). 

Every entry in the config file will be a block of text that must contain the `name` and `path` keys. Each entry is separated by an empty line.

Lines that start with `#` are treated as comments and will be ignored by the config parser. In addition, the first entry is considered as the default entry.

Available keys:
- `name` - The name of the entry which will be shown in the boot menu.
- `path` - The absolute path to the binary which the bootloader is going to load.
- `args` - Arguments which will be passed to the binary.

Let's see an example.

```
name=Arch Linux
path=EFI\Arch\vmlinuz-linux
args=root=UUID=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx rw initrd=EFI\Arch\initramfs-linux.img loglevel=3

name=Windows
path=EFI\Boot\Microsoft\bootmgfw.efi

name=Some UEFI App
path=path\to\UEFI\app
args=args to pass to the app
```

Here we can see 3 entries, each entry is separated by an empty line, and each has the keys `name` and `path`. The key `args` is an optional key which passes arguments to the binary specified in `path`.

## Global configuration

These are special keys that you can put anywhere in the config file and they will change runtime configuration in the bootloader.

Available keys:
- `timeout` - Controls the amount of time the bootloader waits before automatically booting the FIRST entry, if no keys are pressed during the count down. Setting the value to `0` will boot the first entry immediately. Setting the value to `-1` will disable the timeout.

#!/bin/bash
# Installs the bootloader into the boot partition and adds a boot entry

if [ "$EUID" -ne 0 ]; then
    echo "This script must be run as root."
    exit 1
fi

mounted_boot_dir=$(lsblk | awk '{print $7}' | grep boot)
lucidloader_dir="${mounted_boot_dir}/EFI/lucidloader"

# Move the bootloader into the created directory
mkdir ${lucidloader_dir} 2>/dev/null
cp lucidloader_x64.efi ${lucidloader_dir} || exit 1
touch ${lucidloader_dir}/lucidloader-config.cfg

# Parses lsblk to get the boot disk + partition
boot_disk="/dev/$(lsblk | grep boot | cut -c7- | cut -d " " -f 1)"
boot_partition="${boot_disk: -1}"
boot_disk="${boot_disk%?}"

# Create the UEFI boot entry, if it doesn't exist already
efibootmgr | grep lucidloader >/dev/null || efibootmgr -c -d $boot_disk -p $boot_partition -L lucidloader -l \\EFI\\lucidloader\\lucidloader_x64.efi >/dev/null

echo "Installation complete! Make sure to configure the bootloader."
echo "The configuration file is located at ${lucidloader_dir}/lucidloader-config.cfg"
echo "Refer to the configuration guide in CONFIGURING.md"
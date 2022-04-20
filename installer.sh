#!/bin/bash
# Installs the boot manager into the boot partition and adds a boot entry

if [ "$EUID" -ne 0 ]; then
    echo "This script must be run as root."
    exit 1
fi

mounted_boot_dir="$(lsblk | awk '{print $7}' | grep '/boot')"
lucidloader_dir="${mounted_boot_dir}/EFI/lucidloader"

# Move the boot manager into the created directory
mkdir ${lucidloader_dir} 2>/dev/null
cp lucidloader_x64.efi ${lucidloader_dir} || exit 1
touch ${lucidloader_dir}/config.cfg

# Parses lsblk to get the boot disk + partition number
full_dev_name="$(findmnt -n ${mounted_boot_dir} | awk '{print $2}')"
boot_disk="/dev/$(lsblk -no pkname ${full_dev_name})"
boot_partition_num="$(partx -g ${full_dev_name} | awk '{print $1}')"

# Create the UEFI boot entry, if it doesn't exist already
efibootmgr | grep lucidloader >/dev/null || efibootmgr -c -d $boot_disk -p $boot_partition_num -L lucidloader -l \\EFI\\lucidloader\\lucidloader_x64.efi >/dev/null

echo "Installation complete! Make sure to configure the boot manager."
echo "The configuration file is located at ${lucidloader_dir}/config.cfg"
echo "Refer to the configuration guide in CONFIGURING.md"

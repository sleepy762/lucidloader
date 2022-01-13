#!/bin/bash
# Installs the bootloader into the boot partition and adds a boot entry

# Create the .efi binary of the bootloader
echo "Compiling the bootloader..."
make -j4 >/dev/null

if [ "$EUID" -ne 0 ]; then
    echo "This script must be run as root."
    exit 1
fi

echo "Creating the directories and files..."
mkdir /boot/EFI/ezboot 2>/dev/null
mv ezboot_x64.efi /boot/EFI/ezboot
touch /boot/EFI/ezboot/config.cfg

echo "Getting boot partition info..."
# Parses lsblk to get the boot disk + partition
boot_disk="/dev/$(lsblk | grep boot | cut -c7- | cut -d " " -f 1)"
boot_partition="${boot_disk: -1}"
boot_disk="${boot_disk%?}"

echo "Creating boot entry..."
# Create the UEFI boot entry
efibootmgr | grep ezboot >/dev/null || efibootmgr -c -d $boot_disk -p $boot_partition -L ezboot -l \\EFI\\ezboot\\ezboot_x64.efi >/dev/null

echo "Installation complete! Make sure to configure the bootloader."
echo "The configuration file is located at /boot/EFI/ezboot/config.cfg"

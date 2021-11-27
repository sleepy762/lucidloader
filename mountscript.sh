#!/bin/bash
# This script mounts the fat image onto a folder in order to browse the files and directories

# Make sure we are running with root access (in order for the mounting to succeed)
if [ "$EUID" -ne 0 ]; then
    echo "Root privilege required."
    exit 1
fi

# Make sure the mounting folder exists
[ -d mount ] || mkdir mount

# Make sure nothing is mounted on the folder
umount mount 2>/dev/null

# Copy the fat image from the ISO and paste it in the root directory
mount cdimage.iso mount
cp mount/fat.img .
umount mount

# Mount the fat image
mount fat.img mount

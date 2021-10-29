#!/bin/sh
# This script mounts the fat image onto a folder in order to browse the files and directories

# Make sure the mounting folder exists
[ -d mount ] || mkdir mount

# Make sure nothing is mounted on the folder
sudo umount mount 2>/dev/null

# Copy the fat image from the ISO and paste it in the root directory
sudo mount cdimage.iso mount
cp mount/fat.img .
sudo umount mount

# Mount the fat image
sudo mount fat.img mount
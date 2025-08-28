#!/bin/bash

CUR_PATH=$(pwd)
EDK2_DIR="$HOME/edk2"
DISK_IMG="$CUR_PATH/disk.img"
MOUNT_POINT="$CUR_PATH/mnt"

echo "Mounting disk image..."
mkdir -p "$MOUNT_POINT"
sudo mount -o loop "$DISK_IMG" "$MOUNT_POINT"

echo "Opening new terminal for disk shell..."
x-terminal-emulator -e bash -c "cd '$MOUNT_POINT'; PS1='diskimg> '; exec bash" &

# 元のシェルではアンマウント待機
read -p "Press ENTER after closing the disk shell terminal to unmount..."
sudo umount "$MOUNT_POINT"
echo "Disk image unmounted."
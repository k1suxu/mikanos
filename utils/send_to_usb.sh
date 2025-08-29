#!/bin/bash

CUR_PATH=$(pwd)

sudo mkdir -p /mnt/usbmem
sudo mount -t drvfs D: /mnt/usbmem # D:の部分は適切なドライレターに変更する
sudo mkdir -p /mnt/usbmem/EFI/BOOT
sudo cp $HOME/edk2/Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi /mnt/usbmem/EFI/BOOT/BOOTX64.EFI
sudo cp $CUR_PATH/../kernel/kernel.elf /mnt/usbmem/kernel.elf
sudo umount /mnt/usbmem
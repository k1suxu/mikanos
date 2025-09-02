#!/bin/bash

set -e # 失敗したら止まるようにしたい

CUR_PATH=$(pwd)
EDK2_DIR="$HOME/edk2"
KERNEL_PATH="$CUR_PATH/../kernel"

# --- kernel.elfの生成 --- 
cd "$KERNEL_PATH"
make

# --- QEMU を別ターミナルで起動 ---
cd "$EDK2_DIR"
source edksetup.sh
build
cd "$CUR_PATH"
$HOME/osbook/devenv/run_qemu.sh $HOME/edk2/Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi $KERNEL_PATH/kernel.elf

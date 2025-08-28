#!/bin/bash

CUR_PATH=$(pwd)
EDK2_DIR="$HOME/edk2"

# --- QEMU を別ターミナルで起動 ---
cd "$EDK2_DIR"
source edksetup.sh
build
cd "$CUR_PATH"
$HOME/osbook/devenv/run_qemu.sh $HOME/edk2/Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi


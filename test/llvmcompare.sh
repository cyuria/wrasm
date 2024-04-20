#!/usr/bin/bash -e

function cleanup {
  rm -f flatwrasm flatllvmmc
  rm -f wrasm.o llvmmc.o
  rm -f wrasm.hex llvmmc.hex
}

trap cleanup EXIT

$1 "$2" -o wrasm.o
llvm-mc --triple=riscv64-unknown-elf -filetype=obj "$2" -o llvmmc.o

llvm-objcopy -O binary wrasm.o flatwrasm
llvm-objcopy -O binary llvmmc.o flatllvmmc

xxd flatwrasm > wrasm.hex
xxd flatllvmmc > llvmmc.hex

diff wrasm.hex llvmmc.hex


#!/usr/bin/bash

set -e

wrasm=$1
asmfile=$2

function cleanup {
  rm -f "${asmfile}.wrasm.o" "${asmfile}.llvmmc.o"
  rm -f "${asmfile}.flatwrasm" "${asmfile}.flatllvmmc"
}

trap cleanup EXIT

${wrasm} "${asmfile}" -o "${asmfile}.wrasm.o"
llvm-mc --triple=riscv64-unknown-elf -filetype=obj "${asmfile}" -o "${asmfile}.llvmmc.o"

llvm-objcopy -O binary "${asmfile}.wrasm.o" "${asmfile}.flatwrasm"
llvm-objcopy -O binary "${asmfile}.llvmmc.o" "${asmfile}.flatllvmmc"

diff <(xxd "${asmfile}.flatwrasm") <(xxd "${asmfile}.flatllvmmc")


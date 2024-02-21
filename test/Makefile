
.PHONY: compare compile wrasm llvmmc printwrasm printllvmmc clean

all: compare clean

compare: printwrasm printllvmmc
	diff wrasm.hex llvmmc.hex

compile: wrasm llvmmc

wrasm:
	../bin/wrasm w2.S -o wrasm.o
	riscv64-unknown-elf-objcopy -O binary wrasm.o workingwrasm

llvmmc:
	llvm-mc --triple=riscv64-unknown-elf -filetype=obj w2.S -o llvmmc.o
	llvm-objcopy -O binary llvmmc.o workingllvmmc

printwrasm: wrasm
	unbuffer xxd -e -g 4 workingwrasm | tee wrasm.hex

printllvmmc: llvmmc
	unbuffer xxd -e -g 4 workingllvmmc | tee llvmmc.hex

clean:
	rm wrasm.o llvmmc.o workingwrasm workingllvmmc wrasm.hex llvmmc.hex

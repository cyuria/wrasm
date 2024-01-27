
.PHONY: compare compile wrasm gas printwrasm printgas clean

all: compare clean

compare: printwrasm printgas
	-diff -u gas.hex wrasm.hex

compile: wrasm gas

wrasm:
	../bin/wrasm -i w2.S -o workingwrasm

gas:
	riscv64-unknown-elf-as -march rv64i w2.S
	riscv64-unknown-elf-objcopy -O binary a.out workinggas

printwrasm: wrasm
	unbuffer xxd -e -g 4 workingwrasm | tee wrasm.hex

printgas: gas
	unbuffer xxd -e -g 4 workinggas | tee gas.hex

clean:
	rm a.out workingwrasm workinggas wrasm.hex gas.hex



.PHONY: compare compile wrasm gas printwrasm printgas clean

all: compare clean

compare: printwrasm printgas
	-diff wrasm.hex gas.hex

compile: wrasm gas

wrasm:
	../bin/wrasm w2.S -o b.out
	riscv64-unknown-elf-objcopy -O binary b.out workingwrasm

gas:
	riscv64-unknown-elf-as -march rv64i w2.S
	riscv64-unknown-elf-objcopy -O binary a.out workinggas

printwrasm: wrasm
	unbuffer xxd -e -g 4 workingwrasm | tee wrasm.hex

printgas: gas
	unbuffer xxd -e -g 4 workinggas | tee gas.hex

clean:
	rm a.out b.out workingwrasm workinggas wrasm.hex gas.hex

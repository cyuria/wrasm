
.PHONY: compare compile wrasm gas printwrasm printgas clean

compare: printwrasm printgas clean

compile: workingwrasm workinggas

wrasm:
	../bin/wrasm -a working.S -b workingwrasm

gas:
	riscv64-unknown-elf-as -march rv64i working.S
	riscv64-unknown-elf-objcopy -O binary a.out workinggas

printwrasm: wrasm
	@echo
	@echo
	hexdump workingwrasm
	@echo
	@echo

printgas: gas
	@echo
	@echo
	hexdump workinggas
	@echo
	@echo

clean:
	-@rm a.out workingwrasm workinggas



.PHONY: compare compile wrasm gas printwrasm printgas clean

compare: printwrasm printgas clean

compile: workingwrasm workinggas

wrasm:
	../bin/wrasm -a w2.S -b workingwrasm -v

gas:
	riscv64-unknown-elf-as -march rv64i w2.S
	riscv64-unknown-elf-objcopy -O binary a.out workinggas

printwrasm: wrasm
	@echo
	@echo
	xxd -e -g 4 workingwrasm
	@echo
	@echo

printgas: gas
	@echo
	@echo
	xxd -e -g 4 workinggas
	@echo
	@echo

clean:
	-@rm a.out workingwrasm workinggas


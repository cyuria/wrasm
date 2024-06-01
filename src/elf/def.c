
#include "elf/def.h"

#define ELF_IDENT                                                         \
	0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, \
		0x00, 0x00, 0x00, 0x00, 0x00

struct elf64header new_elf64header(void)
{
	return (struct elf64header){
		.ident = { ELF_IDENT },
		.type = 0x01, /* Relocatable file */
		.machine = 0xF3, /* RISC-V */
		.version = 1, /* ELF version 1 */
		.flags = 0x0004, /* RISC-V float abi double flag */
		.headersize = sizeof(struct elf64header),
	};
}

struct elf64sectionheader new_elf64sectionheader(void)
{
	return (struct elf64sectionheader){
		.size = sizeof(struct elf64sectionheader),
		.addr = 0x0,
	};
}

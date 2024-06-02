#pragma once

#include <stdint.h>

#define ELF_IDENTSIZE 16

struct elf64header {
	unsigned char ident[ELF_IDENTSIZE];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint64_t entry;
	uint64_t phoffset;
	uint64_t shoffset;
	uint32_t flags;
	uint16_t headersize;
	uint16_t phentrysize;
	uint16_t phcount;
	uint16_t shentrysize;
	uint16_t shcount;
	uint16_t shstrindex;
};

struct elf64sectionheader {
	uint32_t name;
	uint32_t type;
	uint64_t flags;
	uint64_t addr;
	uint64_t offset;
	uint64_t size;
	uint32_t link;
	uint32_t info;
	uint64_t addralign;
	uint64_t entrysize;
};

struct elf64header new_elf64header(void);
struct elf64sectionheader new_elf64sectionheader(void);

struct elf64sym {
	uint32_t name;
	unsigned char info;
	unsigned char other;
	uint16_t shndx;
	uint64_t value;
	uint64_t size;
};

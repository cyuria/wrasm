#pragma once

/* TODO: implement assembler directives */
int parse_directive(char *);

int get_section_by_name(const char *);

int parse_asciz(const char *);
int parse_ascii(const char *);
int parse_section(const char *);


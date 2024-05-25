# Adding Support for a RISC-V Extension

It's always great to support more extensions, but we want the codebase to
actually work. To that effect, here's a simple checklist you can follow to get
started.

## Read Our Contributing Docs

Have a look through the following
* [`docs/contrib/readme.md`](./readme.md)

## Which Files Do I Need to Change?

If you were implementing the RV32I base instruction set, you'd need to create
the following two files:
* [`h/form/rv32i.h`](https://github.com/cyuria/wrasm/h/form/rv32i.h)
* [`src/form/rv32i.c`](https://github.com/cyuria/wrasm/src/form/rv32i.c)

Make sure to replace the filenames with the equivalent those for whichever
extension you are implementing.

You should also have a look through the definitions in these header files:
* [`h/form/generic.h`](https://github.com/cyuria/wrasm/h/form/generic.h)
* [`h/form/instruction.h`](https://github.com/cyuria/wrasm/h/form/instruction.h)
* [`h/macro.h`](https://github.com/cyuria/wrasm/h/macro.h)
* [`h/debug.h`](https://github.com/cyuria/wrasm/h/debug.h)

## I Need More Help

If you need more help, I strongly suggest reading through the code in
[`src/form/rv32i.c`](https://github.com/cyuria/wrasm/src/form/rv32i.c.)

If that is still not enough, just ask someone. You should have an open issue
for adding the instruction set, ask someone there. If you don't then open an
issue. You can also ask on a PR if you've opened one. The only thing we ask
that you don't do is ask questions on an unrelated issue or PR.

## Further Resources

If you want some more resources to learn about RISC-V, I can recommend the non
ISA specifications, which can be found here [riscv-non-isa][2].

Specifically these in relation to wrasm development
* [riscv-non-isa/riscv-asm-manual](https://github.com/riscv-non-isa/riscv-asm-manual)
* [riscv-non-isa/riscv-elf-psabi-doc](https://github.com/riscv-non-isa/riscv-elf-psabi-doc)
* [riscv-non-isa/riscv-toolchain-conventions](https://github.com/riscv-non-isa/riscv-toolchain-conventions)

[2]: https://github.com/riscv-non-isa

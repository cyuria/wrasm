# The wrasm Assembler

Wrasm is a RISC-V assembler, primarily targeting riscv64.

The assembler is currently under development, but is planned to have the
following features:
*   Support for GNU and LLVM assembler features
*   A powerful, Turing complete preprocessor inspired by
    [nasm](https://nasm.us/)
*   Cross platform support for windows, Linux and macOS
    *   currently only x86_64 and riscv64 GNU Linux has been fully tested, but
        CI builds and testing succeeds for windows and macOS[^1].
*   Binaries for as many platforms as have C compilers (probably not, but I can
    dream)

[^1]: CI testing builds wrasm and runs the unit tests, which can be found under
    `test/unit`. CI testing also runs all other automated tests under x86_64
    Ubuntu

## Copyright

Wrasm is Copyright (C) 2024 Cyuria.

The Argtable3 Library is Copyright (C) 1998-2001,2003-2011 Stewart Heitmann.
Parts are Copyright (C) 1989-1994, 1996-1999, 2001, 2003 Free Software
Foundation, Inc.

See [LICENSE](LICENSE) for more information.

See [the Argtable3 LICENSE](subprojects/argtable3/LICENSE) for more information.


# The wrasm Assembler

Wrasm is a RISC-V assembler, primarily targeting riscv64.

The assembler is currently under development, but is planned to have the
following features:
- Full support for GNU assembler features
- A powerful, Turing complete preprocessor inspired by [nasm](https://nasm.us/)
- Cross platform support for windows, Linux and macOS
  - currently only x86_64 GNU Linux has been tested, but CI builds succeed for
    windows and macOS, automated testing using the LLVM toolchain will be
    implemented once it works with the elf binaries produced by the assembler,
    currently LLVM segfaults.
- Binaries for as many platforms as have C compilers (probably not, but I can
  dream)

## Compiling

To compile wrasm, ensure you have [meson](https://mesonbuild.com) as well as
[ninja](https://ninja-build.org) and a compatible C compiler installed on your
system.

To compile run the following commands in a terminal or command prompt:
```sh
meson setup build
cd build
meson compile
```

When recompiling, it is only necessary to run
```sh
meson compile
```

If compiling from a different directory, run
```sh
meson compile -C path/to/build/directory
```

## Installing

If you downloaded wrasm pre-built, you must install it manually. Simply copy
the `wrasm` executable to `/usr/local/bin` on Unix or to
`C:\Program Files\wrasm\bin` on Windows and ensure the executable is on your
system path.

```sh
meson install
```

When building wrasm, you can run the command `meson install` from the build
directory to automatically install the final executable to `/usr/local/bin` on
Unix or `C:\Program Files\wrasm\bin` on Windows (Note this requires elevated
priveledges). Please ensure the `bin` directory to which the `wrasm` executable
was installed is on your system path. This will need to be done when installing
on Windows or in a non-standard location on Unix based systems.

## Copyright

Wrasm is Copyright (C) 2024 Cyuria. Parts are Copyright (C) 1998-2001,2003-2011
Stewart Heitmann. Parts are Copyright (C) 1989-1994, 1996-1999, 2001, 2003 Free
Software Foundation, Inc.

See [LICENSE](LICENSE) for more information.


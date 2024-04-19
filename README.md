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

To compile wrasm, ensure you have [CMake](https://cmake.org/) (version 3.24 or newer) as well as a
compatible C compiler installed on your system.

To compile run the following commands in a terminal or command prompt:
```sh
cmake . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --config RelWithDebInfo
```

You may add or change any CMake flags as you see fit (such as building debug
or release configurations), however do keep in mind that there is a provided
build directory which maintainers will be expected to use.

### The Makefile

There is a [Makefile](./Makefile) provided for your convenience that will call
CMake for debug or release automatically, simply run one of `make debug` or
`make release` to build the respective targets.

## Installing

If you have built wrasm yourself, you can run `cmake --install .` from the
build directory to automatically install the final executable to
`/usr/local/bin` on Unix or `C:\Program Files\wrasm\bin` on Windows.

If you have downloaded wrasm from the releases page (will be available once
wrasm is functional), you will need to install the executable manually.

Please ensure the `bin` folder into which wrasm was installed is on the system
path, as otherwise you will not be able to use the executable.

## Copyright

Wrasm is Copyright (C) 2024 Cyuria. Parts are Copyright (C) 1998-2001,2003-2011
Stewart Heitmann. Parts are Copyright (C) 1989-1994, 1996-1999, 2001, 2003 Free
Software Foundation, Inc.

See [LICENSE](LICENSE) for more information.


# The wrasm Assembler

Wrasm is a RISC-V assembler, primarily targeting riscv64.

The assembler is currently under development, but is planned to have the
following features:
- Full support for GNU assembler features
- A powerful, Turing complete preprocessor inspired by [nasm](https://nasm.us/)
- Cross platform support for windows, Linux and macOS (currently only x86_64
  GNU Linux has been tested, but CI builds succeed for windows and macOS)
- Binaries for many if not all supported platforms

## Compiling

To compile for most people, install the [requirements](#requirements) listed
for your system and run `make` from the root directory.

This should result in the creation of a `wrasm` or `wrasm.exe` file in the bin
directory.

### Requirements

The build system for wrasm uses Unix makefiles. All systems require a Unix
compatible make to be installed.

The default compiler used and tested by the CI system is clang, which must be
on path unless using a different compiler. To override the compiler, set the
`cc` variable when calling make
```sh
make cc=/path/to/cc
```

#### Linux Requirements

There are no extra requirements for building Linux

#### Windows Requirements

Compiling on Windows requires clang (or another `gcc` compatible compiler) and
a Unix compatible make. It may be possible to compile with the MSVC toolchain,
but it has not been tested and support is not planned at any point.

Binaries for each required program are available below:

- Clang from [the LLVM project](https://github.com/llvm/llvm-project/releases/latest)
- Make[^1] from [GNUwin32](https://gnuwin32.sourceforge.net/install.html)

Once these are available on path, simply execute the `make` command in the root
project directory.

[^1]: Any Unix compatible make works, alternatives include `make` from Cygwin,
    MSYS, MinGW or any other Unix compatible `make`. It is also possible to
    download GNUwin32 make directly

#### macOS Requirements

The build system uses clang by default. Clang is Apple's default compiler and
should be built in with Apple Xcode.

Because the build system is Unix make based, `make` needs to be installed. The
GNU toolchain, which contains make, should be available from
<https://developer.apple.com/>

## Copyright

Wrasm is Copyright (C) 2024 Cyuria. Parts are Copyright (C) 1998-2001,2003-2011
Stewart Heitmann. Parts are Copyright (C) 1989-1994, 1996-1999, 2001, 2003 Free
Software Foundation, Inc.

See [LICENSE](LICENSE) for more information.


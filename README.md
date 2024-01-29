# The wrasm Assembler

Wrasm is a RISC-V assembler, primarily targeting riscv64.

The assembler is currently under development, but is planned to have the
following features:
- Full support for GNU assembler features
- A powerful, Turing complete preprocessor inspired by [nasm](https://nasm.us/)
- Cross platform support for windows, Linux and macOS (currently only windows
and x86_64 GNU Linux have been tested)
- Binaries for many if not all supported platforms

## Compiling

To compile for most people, install the [requirements](#requirements) listed
for your system and run `make` from the root directory.

This should result in the creation of a `wrasm` or `wrasm.exe` file in the bin
directory.

### Requirements

The build system uses Unix make and clang by default to compile. These are
required on all systems.

All systems compile with clang by default. To override this behaviour, set the
`cc` variable when calling make
```sh
make cc=/path/to/cc
```

#### Linux Requirements

Linux requires the `clang` executable on the path, as well as any Unix
compatible `make`. If you do not have the `"argp.h"` header, you may need to
either install an available `argp-standalone` system package or compile with
`noargp`, see [compiling without glibc or `argp.h` on
Linux](#compiling-without-glibc-or-argph-on-linux)

#### Windows Requirements

Compiling on Windows requires CMake, clang and a Unix compatible make. It may
be possible to compile with MSVC, but it has not been tested and support is not
planned at any point.

Binaries for each package are available below:

- CMake from [the CMake website](https://cmake.org/download/#latest)
- Clang from [the LLVM
project](https://github.com/llvm/llvm-project/releases/latest)
- Make[^1] from [GNUwin32](https://gnuwin32.sourceforge.net/install.html)

Once these are available on path, simply execute `make` in the root project
directory.

[^1]: Any Unix compatible make works, alternatives include make from Cygwin,
MSYS, MinGW or any other Unix make compatible make. It is also possible to
download GNUwin32 make directly

#### macOS Requirements

The build system uses clang by default. Clang is Apple's default compiler and
should be built in with Apple Xcode.

The package `argp-standalone` is also required. Install it with
```sh
brew install argp-standalone
```

Because the build system is Unix make based, `make` needs to be installed. The
GNU toolchain, which contains make, should be available from
<https://developer.apple.com/>

### Compiling without glibc or `argp.h` on Linux

If you are on a Linux system without the `"argp.h"` header (provided with
glibc or any `argp-standalone` package that may be available on your system),
the `noargp` variable must be set. This can be done by calling `make noargp=y`.

Compiling with `noargp` requires the `argp-standalone` submodule. This can be
cloned with the following command if it has not already been cloned.
```sh
git submodule update --init 
```

If you use a system `argp-standalone` package or equivalent, compile with the
`libargppath` variable set to the path containing `libargp.a` and the
`libargpinclude` variable set to the path containing `argp.h` if it is not
already available to the compiler. If `libargppath` is already a standard
search location (such as `/usr/lib`), you can set `uselibargp` e.g. one of the
following
```sh
make uselibargp=y
make libargppath=/usr/local/lib
make libargppath=/path/to/argp-standalone/libe \
  libargpinclude=/path/to/argp-standalone/include
```

### A Note on Windows and macOS without glibc or `argp.h`

On Windows, `noargp` is automatically set. The `argp-standalone` submodule does
need to be cloned. This can be done with
```sh
git submodule update --init
```

On macOS, `libargppath` is automatically set to
`/usr/local/opt/argp-standalone/lib` and `libargpinclude` is set similarly.
Both are overridable from the command line, see [Compiling without glibc or
`argp.h` on Linux](#compiling-without-glibc-or-argph-on-linux).

## Copyright

Wrasm is Copyright (C) 2024 Cyuria. Parts are Copyright (C) 1998-2001,2003-2011
Stewart Heitmann. Parts are Copyright (C) 1989-1994, 1996-1999, 2001, 2003 Free
Software Foundation, Inc.

See [LICENSE](LICENSE) for more information.


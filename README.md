# The wrasm Assembler

Wrasm is a RISC-V assembler, primarily targeting riscv64.

The assembler is currently under development, but is planned to have the
following features:
*   Full support for GNU assembler features
*   A powerful, Turing complete preprocessor inspired by [nasm](https://nasm.us/)
*   Cross platform support for windows, Linux and macOS
    *   currently only x86_64 and riscv64 GNU Linux has been fully tested, but
        CI builds and testing succeeds for windows and macOS[^1].
*   Binaries for as many platforms as have C compilers (probably not, but I can
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
privileges). Please ensure the `bin` directory to which the `wrasm` executable
was installed is on your system path. This will need to be done when installing
on Windows or in a non-standard location on Unix based systems.

## Testing

### Unit Tests

To run all the unit tests, simply run the following in the build directory
```sh
meson test
```

### Other Tests

There are also a number of other tests which may also be run, however these use
the compiled wrasm binary. To run these, run the python script located in
`test/system/runtest.py`. Running this script requires LLD (the LLVM/clang
linker) and riscv64 QEMU Userspace emulation binaries installed[^2].

For most people, after wrasm has been built, the script can simply be run as
follows:
```sh
python3 test/system/runtests.py
```
or even just
```sh
test/system/runtests.py
```

Some people may encounter issues with the automatic detection of a program. In
that case, see the following subsections for how they are found and how to
manually specify each of them.

Note that every below command expects that you are using the bourne shell or a
derivative thereof (i.e. bash, zsh, etc), because of this, you may need to
modify the command if you are using a different shell (i.e. PowerShell).

#### Finding Wrasm

The testing script attempts to search for an executable called `wrasm`,
starting from the root of the project (located via the existence of a `.git`
folder) and continuing upward into all subdirectories. If the wrong or no
executable is found, it is manually specifiable via the `WRASM` environment
variable. This can be specified as follows:
```sh
WRASM=build/wrasm python3 test/system/runtests.py
```

#### Finding LLD

The testing script attempts to find the relevant LLD executable on the path.
On Windows it searches for `lld-link`, on macOS it searches for `ld64.lld` and
on Linux systems it searches for `ld.lld`. If the script is unable to correctly
locate the LLD binaries on your path, it is possible to override it by setting
the LLD environment variable as follows:
```sh
LLD=/usr/bin/ld.lld python3 test/system/runtests.py
```

#### Finding QEMU

The testing script tries to find an executable by one of the following names on
the system path, `qemu-riscv64` or `qemu-riscv64-static`. If the incorrect
binary was found, set the `QEMU_RISCV64` environment variable as follows:
```sh
QEMU_RISCV64=/usr/bin/qemu-riscv64 python3 test/system/runtests.py
```

## Copyright

Wrasm is Copyright (C) 2024 Cyuria. Parts are Copyright (C) 1998-2001,2003-2011
Stewart Heitmann. Parts are Copyright (C) 1989-1994, 1996-1999, 2001, 2003 Free
Software Foundation, Inc.

See [LICENSE](LICENSE) for more information.

[^1]: CI testing builds wrasm and runs the unit tests, which can be found under
    `test/unit`. CI testing also runs all other automated tests under x86_64
    Ubuntu
[^2]: See [qemu.org](https://www.qemu.org) and
    [QEMU User space emulator](https://www.qemu.org/docs/master/user/main.html)


# Testing Wrasm

To test wrasm, you must [build from source](build.md).

## Unit Tests

To run all the unit tests, simply run the following in the build directory
```sh
meson test
```
or
```sh
meson test -C path/to/build
```

## Other Tests

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
if you are on Unix (or a Unix like system, it *might* work on MSYS).

Some people may encounter issues with the automatic detection of a program. In
that case, see the following subsections for how they are found and how to
manually specify each of them.

Note that every below command expects that you are using the Bourne shell or a
derivative thereof (i.e. bash, zsh, etc), because of this, you may need to
modify the command if you are using a different shell (i.e. PowerShell).

[^2]: See [qemu.org](https://www.qemu.org) and
    [QEMU User space emulator](https://www.qemu.org/docs/master/user/main.html)

### Finding Wrasm

The testing script attempts to search for an executable called `wrasm`,
starting from the root of the project (located via the existence of a `.git`
folder) and continuing upward into all subdirectories. If the wrong or no
executable is found, it is manually specifiable via the `WRASM` environment
variable. This can be specified as follows:
```sh
WRASM=build/wrasm python3 test/system/runtests.py
```

### Finding LLD

The testing script attempts to find the relevant LLD executable on the path.
On Windows it searches for `lld-link`, on macOS it searches for `ld64.lld` and
on Linux systems it searches for `ld.lld`. If the script is unable to correctly
locate the LLD binaries on your path, it is possible to override it by setting
the LLD environment variable as follows:
```sh
LLD=/usr/bin/ld.lld python3 test/system/runtests.py
```

### Finding QEMU

The testing script tries to find an executable by one of the following names on
the system path, `qemu-riscv64` or `qemu-riscv64-static`. If the incorrect
binary was found, set the `QEMU_RISCV64` environment variable as follows:
```sh
QEMU_RISCV64=/usr/bin/qemu-riscv64 python3 test/system/runtests.py
```


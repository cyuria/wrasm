# Building Wrasm

To compile wrasm, ensure you have [meson](https://mesonbuild.com) as well as
[ninja](https://ninja-build.org) and a compatible C[^1] compiler installed on your
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
from the build directory.

To compile from another directory, just add the `-C` option to meson:
```sh
meson compile -C path/to/build/directory
```

[^1]: The C compiler should support the c17 standard (this may be updated to
    c23 once proper compiler support is established). At the time of writing,
    wrasm successfully builds with c99, however support is not guaranteed.

## Choosing a different compiler and linker

It is possible to manually set the C compiler meson uses with the `CC` variable
and the linker with the `CC_LD` or `C_LD` variables.

To do so, you must run `meson setup` with these variables set. This will then
store the compiler for the `meson compile` step.

```sh
CC=~/gcc/bin/gcc CC_LD=/usr/bin/ld meson setup build
cd build
# Uses ~/gcc/bin/gcc instead of default compiler
meson compile
```

### Microsoft's Visual C/C++ Compiler

To use Microsoft's compiler, ensure you are in a Visual Studio developer shell
when running `meson setup`. This should automatically find MSVC for you.

If you have any problems, consult the
[meson documentation on using Visual Studio][1].

[1]: https://mesonbuild.com/Using-with-Visual-Studio.html

## Other Configurations

For more advanced configuration, such as cross compilation, see the
[meson documentation](https://mesonbuild.com/)

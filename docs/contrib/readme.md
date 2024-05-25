# Documentation for Contributors

For guidelines on contributing, also read
[CONTRIBUTING.md](/CONTRIBUTING.md)

## The Spec

The Spec refers to the RISC-V specifications, specifically the latest version.
This can be found at [riscv.org/technical/specifications][1]. No one expects
you to read through the entire specifications, however crucial knowledge of
RISC-V can be found here.

[1]: https://riscv.org/technical/specifications

## Code Style

Wrasm follows the Linux kernel style guidelines, so when in doubt, consult
them.

There are also have a few general purpose headers
*   [`h/xmalloc.h`](https://github.com/cyuria/wrasm/tree/master/h/xmalloc.h)
    adds an exit call if any of the `malloc` family fails. Please use these
    instead of the regular standard library calls, i.e. `xmalloc(size)` instead
    of `malloc(size)`.
*   [`h/stringutil.h`](https://github.com/cyuria/wrasm/tree/master/h/stringutil.h)
    provides a few utilities for parsing strings.
*   [`h/debug.h`](https://github.com/cyuria/wrasm/tree/master/h/debug.h)
    provides all logging functionality. Please don't use `stdio.h` functions
    for logging purposes but instead the `logger()` function defined here.
*   [`h/macros.h`](https://github.com/cyuria/wrasm/tree/master/h/macros.h)
    provides generic macros, such as the `ARRAY_LENGTH()` macro.

## For Specifics on Contributions

* [Adding a New RISC-V Extension](./adding_extensions.md)


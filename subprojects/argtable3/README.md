# Argtable3 within wrasm

Argtable3 is used in wrasm to parse command line arguments in a cross platform
manner. The amalgamation source file can be found under
`subprojects/argtable3/src/` and the public header under
`subprojects/argtable3/h/`.

## Why is it separate?

Simply to keep the codebase for wrasm clean.

By separating argtable3, it becomes more of a library like interface, which
means it can easily be upgraded or changed in the future. It also compiles
separately from the rest of the wrasm codebase, which, for the most part
doesn't actually have any impact but if you wish to manually clean the build
directory without cleaning argtable3, you can do so a bit more easily.

The other main consideration is for licensing. Pretty much it allows people to
easily determine which parts of wrasm are licensed under what. That is,
everything under `subprojects/argtable3` is part of argtable3 and therefore
uses its license. Everything else is part of wrasm and is licensed as such.

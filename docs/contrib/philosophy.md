# The Codebase Philosophy

1. Simple and Suckless
2. Use the latest C standard and only the latest C standard
3. Avoid the use of platform or compiler specific code

## Simple and Suckless

A big inspiration for the project is [suckless](https://suckless.org/). A lot
of the points that could be made here are made on their philosophy page.

In order to keep it simple, each function should do exactly what its name says
and only what its name says. This ensures functions are named correctly for
significantly improved readability and reduces the amount of bloat and excess
code in any one function. Remember, as long as it fits this principle, no code
is too long nor too short to be its own function. The same goes for files. A
file could have three lines (`src/form/csr.h` actually does) or it could have
five thousand lines. As long as everything in the file is logically grouped
together and does the same thing, it's OK, that's what file trees are for.

The other important idea is that simplicity precludes performance. That is, the
simpler the code, the faster it runs. This is not always true, but it is the
case more often than not and in the cases where it isn't true, it is often
still the better option due to compiler optimisation and readability impacts.

On the topic of readability, try to separate interfaces. C isn't object
oriented, but that doesn't mean we can't learn and apply the same principles.
A file should be considered to be one module and the header its interface. If
it gets too large, consider splitting the file into smaller modules or making a
new directory in the source tree. Don't shy away from global variables. They
are often very useful and allow files to be used like modules without passing
fifteen trillion function arguments through equally many functions.

## Use the latest C Standard

The codebase can use anything up to and including c17. At some point in the
future, this is expected to also include c23. Any compiler that does not
support the newest language standard is not officially supported by wrasm. Note
that this does not mean it will not work, in fact as of the time of writing,
any proper compiler supporting c99 should work.

Do not use any extensions or other standards. That means no GNU extensions and
no POSIX extensions. Even if there is for example a windows alternative where
you could write code which functions on every platform. If the functionality is
needed, implement or find a compatibly licensed implementation of the function
and copy it into the codebase.

## Compiler and Platform Specific Code

As stated above, try to avoid this wherever possible. The functions available
in standard library should be more than enough.

Despite all this, there will still be times when some code is compiler
specific. It is your job as a contributor and as the writer of the code to
reduce these as much as possible. A good practice is to define either a helper
function (as seen in `src/main.c`) that handles all the compiler or platform
specific code, or a macro/preprocessor definition which is defined for every
plausible combination of systems.

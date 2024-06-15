# Contributing to wrasm

## Reporting Bugs

If you've found a bug, please open an issue on GitHub.

### Reporting Security Vulnerabilities

The best way to report a security vulnerability is through GitHub's security
page. This allows us to privately try to diagnose and fix the vulnerability. We
would appreciate it if you do NOT open a regular issue for this purpose.

## Docs

If you want to contribute to the docs, please open a pull request on GitHub. If
you are unsure of anything, you can either ask by commenting on said PR, or by
opening/commenting on a related issue.

## Code

> [!NOTE]
> The "wrasm codebase" means the code that is considered a part of wrasm and
> NOT part of any libraries used by wrasm. This basically means anything under
> the `src/`, `h/` and `test/` directories as well as the root `meson.build`
> file.

If you want to contribute any code, whether it is to fix bugs or add features,
we would appreciate it if there is an existing issue for the problem. This
allows us to use issues to track bugs and feature requests, without having to
sort through PRs.

## The RISC-V spec

Also referred to as the Spec, the RISC-V specifications can be found at
[riscv.org/technical/specifications][1]. No one expects you to read through the
entire specifications, however crucial knowledge of RISC-V can be found within
these documents.

We may at some point ask you to look at the Spec. This is not because we think
you are stupid, but because we do not feel like repeating everything already
written down by some very smart people.

Some other information can also be found here at [riscv/riscv-isa-manual][2].

[1]: https://riscv.org/technical/specifications
[2]: https://github.com/riscv/riscv-isa-manual

## Whitespace fixes, code style changes, etc

If your PR is entirely or mostly cosmetic code changes, we most likely won't
accept it. I think the explanation given [here][3] for this is incredibly good.
If you find this kind of problem, please either open an issue about it or
mention it in one such existing issue.

[3]: https://github.com/rails/rails/pull/13771#issuecomment-32746700

### A Long Note on Typos

The above doesn't really apply to typos, so don't worry. Please use your own
judgement in regard to the importance of a PR for the typo.

For docs, we don't want typos, so we'll most likely let the PR through.

For code, it depends on the readability impact of the typo. If that portion of
the code has a minor typo which doesn't impact its readability, just raise an
issue. Hopefully it will get fixed the next time that piece of code is updated.
If the typo severely impacts the code's readability or makes it not perform as
expected, that's a more serious bug, please submit a PR, or at least let us
know with an issue.

Notice how all the solutions start with raising an issue? That's because you
can use the issue to gauge the importance of a corresponding PR. Isn't that
convenient.

## Something Else?

> [!TIP]
> Always open an issue, it's the best way to communicate with others about the
> state of the project without doing too much work.

If there's anything that isn't covered here, the general process for
contributing should be something along the lines of:
1. Open an issue about it
2. Figure out what to do next with a maintainer or whoever
3. Pull Request?

## Design Philosophy

The wrasm codebase is designed to be as independent as possible, this means it
should be compilable on any C compiler for any architecture on any operating
system. In practice, this isn't really feasible to test, but some general
guidelines are:
1.  Use only stdlib features defined in the C standard. I.e. no Linux specific
    headers. For anything else, consider either writing the code yourself, or
    including the source code for a library which does so [^1].
2.  Avoid any undefined behaviour. Even if it works, it's generally not best
    practices and could cause issues on another system.
3.  If any OS/compiler/architecture specific code is absolutely necessary,
    ensure there is a fully functional, compilable alternative that should work
    on every system, even of it's not best practices (also see no. 2 above)
4.  If it's in the C standard, you don't have to worry about it. Part of the
    philosophy is that standards change for a reason and therefore adherence to
    an ancient standard is bad form.

The other main part of the philosophy is to give the end user the power to
shoot themselves in the foot with a warning. In this vein, it is theoretically
possible to compile wrasm on a C compiler that does not define `__clang__`,
`__GNUC__` or `_MSC_VER`. This will however raise a warning, which means
removing the `-werror` flag is required to do so. What this effectively means
is that any time you raise an error, by default wrasm will exit, however this
is possible to change. Pretty much, even if an error occurs, try to make it
work anyway, or at the very least don't break anything else.

> [!NOTE]
> The philosophy only applies to wrasm, not to external components such as
> argtable3

If you have some time to kill and want to read through a big long document
about [@cyuria][4]'s thoughts on code style and readability have a look through
[the best code practices][5].

[4]: https://github.com/cyuria
[5]: https://cyuria.github.io/practices

[^1]: We would like to avoid reliance on libraries where possible, but for
    certain tasks it simply makes sense to use one. In that case please try to
    select a library which allows us to continue to follow the philosophy
    within the wrasm codebase.

## Code Style

See also the [codebase philosophy](./philosophy.md).

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

## Codebase Philosophy

see [PHILOSOPHY](./philosophy.md)

## For Specifics on Contributions

* [Adding a New RISC-V Extension](./adding_extensions.md)


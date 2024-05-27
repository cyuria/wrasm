# Installing Wrasm

## Installing from Pre-Built Binaries

As wrasm is currently in development, there are no packages or provided
binaries. Once wrasm is ready for its first release, it will be available via
the [GitHub releases page](https://github.com/cyuria/wrasm/releases).

If you downloaded wrasm pre-built, you must install it manually. Simply copy
the `wrasm` executable to `/usr/local/bin` on Unix or to
`C:\Program Files\wrasm\bin` on Windows and ensure the executable is on your
system path.

## Installing from Compiled Source

First see [build](build.md) for building wrasm from source.

Once you have built wrasm, run the following from the build directory.
```sh
meson install
```

> [!NOTE]
> You do not need to run the `meson compile` step as meson will automatically
> build wrasm for you if it has not already been done when you try to install
> it.

This will install the `wrasm` executable binary to `/usr/local/bin` on Unix
based systems, or `C:\Program Files\wrasm\bin` on Windows.

> [!NOTE]
> Installing wrasm will require elevated privileges (administrator on Windows
> or sudo on Linux)

### Installing to a different directory

To set the install prefix, run the following command before `meson install`
```
meson configure ./path/to/build/dir --prefix $HOME/.local
```

### Installing Manually

To install wrasm manually, just copy the `wrasm` executable located in the root
of the `build` directory to your installation location of choice.

### Other Installation Configurations

For more complicated install configurations, also see
[the meson docs on installing][1].

This documents features such as `DEST_DIR` support.

[1]: https://mesonbuild.com/Installing.html


# Rust PNG decoder

This directory contains experimental code for providing `SkCodec` API
for decoding PNG images using Rust `png` crate.  See the following
document for more details:
https://docs.google.com/document/d/1glx5ue5JDlCld5WzWgTOGK3wsMErQFnkY5N5Dsbi91Y/edit?usp=sharing

The current arrangements for building this code are temporary and
are subject to change in the future.  In particular, this directory
provides *no* support for Skia standalone build, and only experimental
support for Chromium builds.  For more discussion about the dependency
on Chromium's `//build/rust`, please see:
https://docs.google.com/document/d/1gWRQCziaIuVtZmussVpBR_cauhkMlYmBy54om_hwbZU/edit?usp=sharing

To build and test this directory, run the following steps from the root of a
Chromium repository:

1. Ensure that https://crrev.com/c/4723145 and https://crrev.com/c/5747366 have
   landed or have been cherry-picked to the local repo
1. `gn args out/...` and set `enable_rust_png = true`
1. `autoninja -C out/... gfx_unittests`
1. `out/.../gfx_unittests --gtest_filter=RustEnabled*`

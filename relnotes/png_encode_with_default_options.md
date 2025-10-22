New `SkXPS::MakeDocument` overload allows explicitly specifying which
PNG encoder should be used.  This enables avoiding a hardcoded, transitive
dependency on either `libpng` or Rust PNG.  To ease the transition, two
new `EncodeUsingDefaultOptions` helper functions have been added to the
`SkPngEncoder` and `SkPngRustEncoder` namespaces.

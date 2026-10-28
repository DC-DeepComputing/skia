---
name: skia/rust-align-module-names-with-file-paths
description: 'When declaring Rust modules, keep module names aligned with source file paths using standard mod declarations instead of #[path = "..."] attributes'
rubric: readability
severity: should_fix
---

**Rule.** Do not use #[path = "..."] attributes on Rust module declarations to
point to differently named source files. Keep module names and filesystem file
paths strictly aligned by using standard mod <name>; declarations matching
<name>.rs (or <name>/mod.rs).

**Why.** Using #[path = "..."] to bind a differently named module to a file
creates an unexpected discrepancy between the module hierarchy and the
filesystem layout. Keeping module names aligned with source file paths adheres
to idiomatic Rust conventions and ensures consistent code navigation, tooling
support, and maintainability.

## Examples

Bad:

```cpp
#[path = "exif_parse.rs"]
mod tiff;
```

Good:

```cpp
mod exif_parse;
```

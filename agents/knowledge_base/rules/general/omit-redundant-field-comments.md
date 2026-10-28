---
name: skia/omit-redundant-field-comments
description: When declaring member variables or struct fields, omit comments that merely restate identifiers or types, and document ownership, lifetime, or non-obvious invariants instead
rubric: readability
severity: nit
---

**Rule.** Do not add comments to member variables or struct fields that merely
restate what can be directly inferred from the variable's name and type. Reserve
field comments for explaining non-obvious invariants, ownership architecture and
lifetime contracts across cooperating components, concurrency constraints, or
units of measurement.

**Why.** Comments that duplicate what can be directly inferred from an
identifier's name and type add visual clutter and maintenance overhead without
conveying useful context. High-value field comments explain ownership
architecture, lifetime partitioning across interacting classes, or non-obvious
invariants rather than echoing what the compiler and reader already know.

## Examples

Bad:

```cpp
// Rust decoder instance
rust::Box<rust_bmp::BmpDecoder> fRustDecoder;

// fOwnedBaseCanvas always points to the base canvas, even when capturing.
std::unique_ptr<SkCanvas> fOwnedBaseCanvas = nullptr;
```

Good:

```cpp
rust::Box<rust_bmp::Reader> fReader;

// SkSurface_Base must always own the base canvas. During capture, SkCaptureManager
// owns any wrapping capture canvas that fCachedCanvas may point to.
std::unique_ptr<SkCanvas> fOwnedBaseCanvas = nullptr;
```

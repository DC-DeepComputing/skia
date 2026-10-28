---
name: skia/avoid-speculative-placeholder-files
description: When adding files for new features, do not commit speculative placeholder files or empty headers; defer them until their implementation and build targets are required
rubric: design
severity: should_fix
---

**Rule.** Do not add speculative placeholder files, empty headers, or
unpopulated namespaces for anticipated future features or extensions. Introduce
new source and header files cohesively only when their concrete implementation,
build rules (GN/Bazel), and bindings or callers are actively implemented and
required.

**Why.** Adding mostly empty placeholder files or empty headers introduces
visual clutter and speculative scaffolding before designs are settled.
Introducing files together with their implementation, build targets, and
consumers ensures all components are validated by CI and avoids leaving orphaned
stubs in the tree.

## Examples

Bad:

```cpp
// In experimental/rust_bmp/ffi/FFI.h
// This header is a placeholder for future FFI extensions.
namespace rust_bmp {
// Reserved for future processor interfaces
}
```

Good:

```cpp
// Omit placeholder files and empty headers entirely. Introduce new files
// only when their implementation, build definitions (BUILD.gn / BUILD.bazel),
// and active callers are ready in the change.
```

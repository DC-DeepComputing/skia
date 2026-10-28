---
name: skia/prefix-classes-with-sk-and-match-header-filename
description: When declaring C++ classes, use Sk prefix (or Gr in Ganesh) matching header filenames, but scope backend types in backend namespaces (preferring CPU over Raster)
rubric: readability
severity: should_fix
---

**Rule.** Name generic Skia C++ classes and structs with the standard Sk prefix
and match the primary class name to its header filename (e.g., class
SkStreamAdapter in SkStreamAdapter.h). In the Ganesh subsystem
(src/gpu/ganesh/), use the legacy Gr prefix (e.g., GrBitmap) rather than
introducing skgpu::ganesh. For modern backend-specific types (such as contexts
and recorders in CPU or Graphite), scope classes in dedicated backend namespaces
(e.g., namespace skcpu, namespace skgpu::graphite) without an Sk prefix (e.g.,
skcpu::Recorder), and prefer the modern term 'CPU' / 'cpu' (such as namespace
skcpu) over legacy 'Raster' terminology.

**Why.** Skia designates codebase ownership and avoids global namespace
collisions using the Sk prefix for core types and Gr in legacy Ganesh. Modern
backends like Graphite and CPU isolate backend-specific types into dedicated
lowercase namespaces, avoiding prefix clutter and reserving the Sk prefix for
generic, backend-agnostic abstractions. Furthermore, 'Raster' is legacy
terminology from when Skia was exclusively CPU-based, whereas 'CPU' clearly
distinguishes CPU rendering from modern GPU backends.

## Examples

Bad:

```cpp
// Bad: Unprefixed top-level class diverging from header filename
// In rust/common/SkStreamAdapter.h
class StreamAdapter {
public:
    explicit StreamAdapter(SkStream* stream);
};

// Bad: Using skgpu::ganesh namespace instead of Gr prefix in Ganesh
// In src/gpu/ganesh/image/GrBitmap.h
namespace skgpu::ganesh {
class Bitmap {
public:
    explicit Bitmap(SkBitmap b);
};
}

// Bad: Using Sk prefix and legacy 'Raster' naming for backend-specific types
// In include/core/SkRasterContext.h
class SK_API SkRasterContext {
public:
    std::unique_ptr<SkRasterRecorder> makeRecorder() const;
};
```

Good:

```cpp
// Good: Prefixed with Sk and matching header filename
// In rust/common/SkStreamAdapter.h
class SkStreamAdapter {
public:
    explicit SkStreamAdapter(SkStream* stream);
};

// Good: Gr-prefixed class in Ganesh
// In src/gpu/ganesh/image/GrBitmap.h
class GrBitmap {
public:
    explicit GrBitmap(SkBitmap b);
};

// Good: Modern backend types scoped in dedicated backend namespace using 'cpu'
// In include/cpu/Context.h
namespace skcpu {
class SK_API Context {
public:
    std::unique_ptr<Recorder> makeRecorder() const;
};
}  // namespace skcpu
```

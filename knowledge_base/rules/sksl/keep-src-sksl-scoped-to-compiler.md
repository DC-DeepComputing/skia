---
name: skia/sksl/keep-src-sksl-scoped-to-compiler
description: When implementing core rendering logic or SkDevice/SkCanvas methods that consume SkSL, place them in src/core/ rather than inside src/sksl/
rubric: design
severity: should_fix
---

**Rule.** Keep src/sksl/ strictly scoped to the SkSL compiler and shading
language pipeline (lexer, parser, AST/IR, inlining, and code generators). Do not
place core graphics rendering, device backends, or canvas implementations (such
as SkBitmapDevice or SkCanvas draw methods) that consume SkSL into src/sksl/;
place them in src/core/ alongside related runtime effect features.

**Why.** src/sksl/ is designed as a standalone, target-agnostic shading language
compiler that must build and operate independently of graphics devices and
high-level canvas abstractions. Implementing core rendering logic or device
methods inside src/sksl/ violates modular layering boundaries and entangles
compiler infrastructure with drawing pipelines.

## Examples

Bad:

```cpp
// In src/sksl/SkBitmapDevice_mesh.cpp:
#include "src/core/SkBitmapDevice.h"

void SkBitmapDevice::drawMesh(const SkMesh& mesh,
                              sk_sp<SkBlender> blender,
                              const SkPaint& paint) {
    // Core raster device drawing implementation...
}
```

Good:

```cpp
// In src/core/SkBitmapDevice.cpp:
#include "src/core/SkBitmapDevice.h"

void SkBitmapDevice::drawMesh(const SkMesh& mesh,
                              sk_sp<SkBlender> blender,
                              const SkPaint& paint) {
    // Core raster device drawing implementation...
}
```

---
name: skia/modules/preserve-established-canvaskit-api-methods
description: When modifying or extending CanvasKit Embind bindings, preserve established method bindings and entry points rather than removing, renaming, or accidentally replacing them
rubric: correctness
severity: blocking
---

**Rule.** Do not rename, delete, or inadvertently replace established CanvasKit
public API methods or Emscripten bindings without an intentional deprecation
period. When registering new or specialized methods in Embind
(EMSCRIPTEN_BINDINGS), verify that existing member function bindings (such as
.function(...)) are preserved rather than overwritten, and retain existing
canonical entry points (or alias legacy and new names) when introducing API
variants.

**Why.** CanvasKit adheres to Semantic Versioning and is published directly to
npm for external web applications. Deleting, renaming, or accidentally replacing
established bindings breaks downstream JavaScript/WASM clients immediately upon
upgrading.

## Examples

Bad:

```cpp
// modules/canvaskit/canvaskit_bindings.cpp: Renaming established entry points
class_<SkRuntimeEffect>("RuntimeEffect")
    .smart_ptr<sk_sp<SkRuntimeEffect>>("sk_sp<RuntimeEffect>")
    - .class_function("_Make", optional_override([](...) { ... }))
    + .class_function("_MakeForShader", optional_override([](...) { ... }))
    .class_function("_MakeForBlender", optional_override([](...) { ... }));

// modules/canvaskit/skottie_bindings.cpp: Accidentally replacing existing bindings
class_<ManagedAnimation>("ManagedAnimation")
    - .function("setOpacity", &ManagedAnimation::setOpacity)
    + .function("_setTransform", optional_override([](...) { ... }));
```

Good:

```cpp
// modules/canvaskit/canvaskit_bindings.cpp: Retain established entry point
class_<SkRuntimeEffect>("RuntimeEffect")
    .smart_ptr<sk_sp<SkRuntimeEffect>>("sk_sp<RuntimeEffect>")
    .class_function("_Make", optional_override([](...) { ... }))
    .class_function("_MakeForBlender", optional_override([](...) { ... }));

// modules/canvaskit/skottie_bindings.cpp: Retain existing binding alongside new one
class_<ManagedAnimation>("ManagedAnimation")
    .function("setOpacity", &ManagedAnimation::setOpacity)
    .function("_setTransform", optional_override([](...) { ... }));
```

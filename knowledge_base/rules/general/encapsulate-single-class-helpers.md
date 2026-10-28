---
name: skia/encapsulate-single-class-helpers
description: When creating helpers or implementation classes for a single class or translation unit, encapsulate them in the owning class or .cpp anonymous namespace instead of dedicated headers
rubric: design
severity: should_fix
---

**Rule.** Do not declare standalone helper classes or concrete implementation
classes in dedicated header files when they only serve a single owning class or
translation unit. Encapsulate single-caller helper state, structs, and methods
as private members or nested types within the owning class, or define
translation-unit-local implementation classes inside an anonymous namespace
within the .cpp source file.

**Why.** Exposing single-use helpers or implementation classes in separate
header files unnecessarily expands internal header surface area, triggers
build-system maintenance overhead in GN and Bazel, increases compilation
overhead, and leaks private details that have no external consumers.

## Examples

Bad:

```cpp
// DawnLoadResolveWithDrawHelper.h (dedicated header for single-caller helper):
class DawnLoadResolveWithDrawHelper { ... };

// src/ports/SkFontMgr_fontations_empty.h (dedicated header for single-TU subclass):
class SkFontMgr_Fontations_Empty : public SkFontMgr { ... };
```

Good:

```cpp
// DawnCommandBuffer.h (encapsulated as nested type within owning class):
class DawnCommandBuffer : public CommandBuffer {
private:
    struct ResolveStepEmulationInfo { ... };
};

// src/ports/SkFontMgr_fontations_empty.cpp (defined in anonymous namespace, no .h):
namespace {
class SkFontMgr_Fontations_Empty : public SkFontMgr { ... };
}  // namespace
```

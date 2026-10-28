---
name: skia/api/keep-implementation-comments-in-source
description: When documenting classes, APIs, or methods in headers, keep comments caller-facing and contract-focused, omitting temporal refactoring notes and internal mechanics
rubric: readability
severity: should_fix
---

**Rule.** When documenting classes, functions, capability accessors, constants,
or types in headers (under include/ and src/), keep header comments
caller-facing, concise, and focused on interface contracts, query semantics,
caller-observable guarantees (such as heuristic query behavior or qualifying
capability checks), and preconditions. Do not leak internal implementation
details, step-by-step execution mechanics, branch-specific execution steps, or
temporal refactoring notes (such as 'now writing to...'); keep internal
architecture overviews, detailed branch-specific implementation comments, helper
mechanics, driver workaround rationales, backend-specific driver extensions or
hardware caps initializers, and internal implementation TODOs in source files
(.cpp) adjacent to definitions. When an exposed API is experimental, warn
callers with client-facing comments on the declaration rather than internal
notes.

**Why.** Header files define external and subsystem interface contracts and
should remain concise, stable, and discoverable for callers without leaking
private encapsulation details, transient refactoring notes, or backend driver
workaround mechanics. Placing internal architecture, branch-specific execution
steps, driver extension details, workaround and fallback rationales, and
implementation TODOs in .cpp files keeps internal notes adjacent to code
definitions while ensuring headers convey clean caller contracts.

## Examples

Bad:

```cpp
// In SkCanvasPriv.h (multi-step internal branching commentary cluttering the header):
/**
 * If the paint has an image filter but no mask filter, this will:
 * 1. SaveLayer (with a paint containing the current imagefilter...)
 * 2. Generate the src pixels...
 * 3. Restore the layer created in #1...
 */
class AutoLayerForImageFilter { ... };

// In src/gpu/graphite/PipelineData.h (leaking temporal refactoring note on public method):
// Mimic the type-safe API available in UniformManager, now writing to a single manager.
template <typename T> void write(const T& t) { fUniformManager.write(t); }

// In include/gpu/graphite/Context.h (cluttering public doc comments with backend driver mechanics):
/**
 * For Vulkan implementations, returns true if pipeline cache data could have changed.
 * VK_EXT_pipeline_creation_cache_control eliminates false positives when enabled.
 */
bool hasNewPipelineCacheData() const;
```

Good:

```cpp
// In SkCanvasPriv.h (clean contract) and SkCanvasPriv.cpp (internal branch mechanics):
// In SkCanvasPriv.h:
class AutoLayerForImageFilter { ... };
// In SkCanvasPriv.cpp:
void AutoLayerForImageFilter::addImageFilterLayer(const SkRect* drawBounds) {
    // The restore paint for an image filter layer takes the image filter and blending...
}

// In src/gpu/graphite/PipelineData.h (clean interface contract without refactoring history):
// Mimic the type-safe API available in UniformManager
template <typename T> void write(const T& t) { fUniformManager.write(t); }

// In include/gpu/graphite/Context.h (clean contract with caller-observable guarantees):
/**
 * Returns true if pipeline cache data could have changed since the last sync.
 * When `canDetectNewPipelineCacheData` returns true, false positives are greatly reduced.
 */
bool hasNewPipelineCacheData() const;
```

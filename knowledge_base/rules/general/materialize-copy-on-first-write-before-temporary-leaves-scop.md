---
name: skia/materialize-copy-on-first-write-before-temporary-leaves-scop
description: When initializing an SkTCopyOnFirstWrite with a local or temporary object, call writable() or use initIfNeeded before the source exits scope
rubric: safety
severity: nit
---

**Rule.** When initializing an SkTCopyOnFirstWrite<T> from a stack-allocated
local variable or temporary via .init() or the constructor, ensure the wrapper
clones the object before the source is destroyed. Call .writable() while the
local source is still in scope, construct in-place with .initIfNeeded(...), or
ensure the referenced source object outlives the wrapper.

**Why.** SkTCopyOnFirstWrite defers cloning and only stores a borrowed raw
pointer (const T*) to the source object until .writable() is invoked. If the
referenced stack variable or temporary exits its scope before .writable() is
called, the wrapper retains a dangling pointer, causing use-after-free memory
corruption on subsequent accesses.

## Examples

Bad:

```cpp
SkTCopyOnFirstWrite<RenderContext> fCtx;
if (!fCtx.get()) {
    RenderContext default_ctx;
    fCtx.init(default_ctx);
}
// BUG: default_ctx is destroyed; fCtx holds a dangling pointer.
return fCtx.writable();
```

Good:

```cpp
SkTCopyOnFirstWrite<RenderContext> fCtx;
if (!fCtx.get()) {
    RenderContext default_ctx;
    fCtx.init(default_ctx);
    // Force copy while default_ctx is still in scope.
    return fCtx.writable();
}
return fCtx.writable();
```

---
name: skia/initialize-working-copy-for-conditional-mutations
description: When conditionally modifying small descriptor or context structs, initialize a working copy upfront rather than using SkTCopyOnFirstWrite or ternary reference aliasing
rubric: design
severity: should_fix
---

**Rule.** When a small descriptor, options struct, or context object requires
conditional field modifications, initialize the working instance directly by
value upfront and mutate its fields in place. Do not use copy-on-first-write
wrappers (such as SkTCopyOnFirstWrite) or ternary reference aliasing over
default-constructed temporaries for lightweight, cheap-to-copy types.

**Why.** Small value objects and descriptor structs are cheap to copy.
Initializing the working copy upfront simplifies control flow into a single
mutable variable and simple conditionals, eliminating unnecessary wrapper
abstraction, convoluted deferred-initialization logic, and awkward ternary
reference aliasing.

## Examples

Bad:

```cpp
// Bad: Default-constructing a temporary and selecting via ternary reference
RenderPassDesc compatibleDesc;
if (compatibleOnly) {
    compatibleDesc = originalDesc;
    compatibleDesc.fColorAttachment.fLoadOp = LoadOp::kDiscard;
}
const RenderPassDesc& renderPassDesc = compatibleOnly ? compatibleDesc : originalDesc;

// Bad: Over-engineering copy-on-write wrapper for a lightweight context struct
SkTCopyOnFirstWrite<RenderContext> fCtx;
RenderContext* writableContext() {
    if (!fCtx.get()) {
        fCtx.init(RenderContext());
    }
    return fCtx.writable();
}
```

Good:

```cpp
// Good: Initialize working copy directly from original and mutate in place
RenderPassDesc renderPassDesc = originalDesc;
if (compatibleOnly) {
    renderPassDesc.fColorAttachment.fLoadOp = LoadOp::kDiscard;
}

// Good: Copy lightweight context struct directly upfront by value
ScopedRenderContext::ScopedRenderContext(SkCanvas* canvas, const RenderContext* ctx)
    : fCanvas(canvas)
    , fCtx(ctx ? *ctx : RenderContext())
    , fRestoreCount(canvas->getSaveCount()) {}
```

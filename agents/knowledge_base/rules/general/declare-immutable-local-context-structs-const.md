---
name: skia/declare-immutable-local-context-structs-const
description: When defining local or ad-hoc context structs passed to read-only callbacks or caching functions, declare the struct instance as const
rubric: readability
severity: should_fix
---

**Rule.** When defining local or ad-hoc context structs to pass state by address
into callbacks or caching utilities (such as findOrCreateCachedProxy or
completion procs), declare the struct instance as const (e.g., const struct
Context { ... } context = { ... }; or const Context context = { ... };). Ensure
that callback functions cast the context parameter to pointer-to-const (const
Context*) rather than mutable pointers when the context data is only read.

**Why.** Marking local context structs const communicates read-only intent to
readers, prevents accidental modification before or during callback execution,
and matches the const-correctness of callback APIs that accept const void*. It
provides compiler-enforced guarantees that neither the caller nor the callback
mutates temporary context state.

## Examples

Bad:

```cpp
// Ad-hoc context struct declared mutable despite only being read:
struct ClipDrawContext {
    const ClipStack::ElementList* fElementList;
    SkIRect fBounds;
} context = { elementList, iBounds };

fRecorder->priv().proxyCache()->findOrCreateCachedProxy(
        fRecorder, bounds.size(), &context,
        [](const void* ctx, SkCanvas* c) {
            const ClipDrawContext* cdc = static_cast<const ClipDrawContext*>(ctx);
            draw_clip(c, cdc->fElementList, cdc->fBounds);
        });
```

Good:

```cpp
// Context struct declared const to enforce immutability:
const struct ClipDrawContext {
    const ClipStack::ElementList* fElementList;
    SkIRect fBounds;
} context = { elementList, iBounds };

fRecorder->priv().proxyCache()->findOrCreateCachedProxy(
        fRecorder, bounds.size(), &context,
        [](const void* ctx, SkCanvas* c) {
            const ClipDrawContext* cdc = static_cast<const ClipDrawContext*>(ctx);
            draw_clip(c, cdc->fElementList, cdc->fBounds);
        });
```

---
name: skia/document-redundant-control-flow-lifetimes
description: When duplicate statements, early returns, or seemingly redundant control flow exist to satisfy stack object lifetime constraints, document the rationale in an explanatory comment
rubric: readability
severity: nit
---

**Rule.** When seemingly redundant control flow, early returns, or duplicate
statements exist to satisfy object lifetime constraints—such as forcing a copy
or evaluation while a stack-allocated fallback remains in scope—document the
rationale with an explanatory comment. Explain what temporary object is kept
alive and why the operation cannot be deferred to subsequent shared control
flow.

**Why.** Duplicate statements (such as identical return expressions inside and
after a conditional block) appear redundant and invite well-intentioned
refactorings that consolidate them. If the duplication is intentional to prevent
use-after-free bugs or dangling references to stack-allocated temporaries,
omitting an explanatory comment risks future maintainers inadvertently deleting
the statement and introducing critical memory safety regressions.

## Examples

Bad:

```cpp
if (!fCtx.get()) {
    // TODO: add SkTCopyOnWrite::init_copy()?
    RenderContext default_ctx;
    fCtx.init(default_ctx);
    return fCtx.writable();
}
return fCtx.writable();
```

Good:

```cpp
if (!fCtx.get()) {
    // N.B.: we have to force a copy while the default source is in scope.
    // TODO: add SkTCopyOnWrite::init_copy() to simplify this
    RenderContext default_ctx;
    fCtx.init(default_ctx);
    return fCtx.writable();
}
return fCtx.writable();
```

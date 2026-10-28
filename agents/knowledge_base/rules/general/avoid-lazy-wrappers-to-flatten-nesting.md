---
name: skia/avoid-lazy-wrappers-to-flatten-nesting
description: When conditionally preparing local state, declare plain variables inside nested conditional blocks rather than using SkTLazy or std::optional solely to flatten nesting
rubric: design
severity: should_fix
---

**Rule.** Do not use lazy-initialization or optional wrappers (such as SkTLazy
or std::optional) for local variables solely to flatten conditional control flow
or avoid indenting nested blocks. Declare a standard local variable directly
inside the guarded block where it is used, and nest the subsequent conditional
operations cleanly inside that scope.

**Why.** Wrapping local stack variables in SkTLazy or std::optional purely to
combine sequential conditions into a single compound expression introduces
unnecessary template wrapper overhead, non-idiomatic deferred initialization
(.init() or .emplace()), and extra pointer-like indirection. Using standard
nested conditional blocks keeps variable lifetimes scoped directly to their
point of use and produces idiomatic, readable code.

## Examples

Bad:

```cpp
// Bad: Declaring a lazy wrapper solely to flatten nested conditionals
SkTLazy<SkPaint> layer_paint;

if (isolation && fCtx.modulatePaint(layer_paint.init())) {
    fCanvas->saveLayer(bounds, layer_paint.get());
    fCtx = RenderContext();
}
```

Good:

```cpp
// Good: Plain local variable declared within the nested conditional block
if (isolation) {
    SkPaint layer_paint;
    if (fCtx.modulatePaint(&layer_paint)) {
        fCanvas->saveLayer(bounds, &layer_paint);
        fCtx = RenderContext();
    }
}
```

---
name: skia/image/guard-capture-breakpoints-to-new-snapshots
description: When instrumenting surface image snapshots with capture breakpoints or recorders, trigger the breakpoint only on cache misses when generating a new snapshot
rubric: correctness
severity: should_fix
---

**Rule.** When instrumenting surface snapshot generation with capture
breakpoints or snapshot recording (such as createCaptureBreakpoint()), invoke
the breakpoint only when producing a new snapshot after checking for a cached
image. Do not trigger capture breakpoints unconditionally in entry methods like
makeImageSnapshot() before consulting fCachedImage or refCachedImage().

**Why.** Surfaces cache image snapshots in fCachedImage and return the cached
image if the underlying surface has not been mutated. Invoking capture
breakpoints unconditionally on every snapshot request causes duplicate capture
events and redundant recording overhead on cache hits.

## Examples

Bad:

```cpp
sk_sp<SkImage> SkSurface::makeImageSnapshot() {
    asSB(this)->createCaptureBreakpoint();
    return asSB(this)->refCachedImage();
}
```

Good:

```cpp
sk_sp<SkImage> SkSurface_Base::refCachedImage() {
    if (fCachedImage) {
        return fCachedImage;
    }
    this->createCaptureBreakpoint();

    fCachedImage = this->onNewImageSnapshot();
    return fCachedImage;
}
```

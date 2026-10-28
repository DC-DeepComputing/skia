---
name: skia/verify-base-callers-before-deleting-overrides
description: When removing virtual method overrides during refactoring, check base-class callers to avoid fallback bugs, and delete dead virtuals across the base interface and all subclasses
rubric: correctness
severity: blocking
---

**Rule.** When refactoring or removing virtual method overrides, first verify
callers of the base-class method: do not remove an override if base callers
still invoke it, as default virtual implementations compile cleanly but silently
degrade to fallback behavior at runtime. Conversely, when an overridden method
is truly unreferenced by callers, do not delete the override from only a single
subclass; remove the virtual method from the base interface and delete obsolete
overrides across all sibling subclasses and backends.

**Why.** Deleting an override whose base class provides a non-pure default
implementation causes polymorphic callers to fall back to default or no-op
behavior at runtime, causing functional bugs or performance regressions.
Conversely, removing an unused override from only one backend leaves dead
virtual declarations in the base interface and orphan implementations across
other subclasses.

## Examples

Bad:

```cpp
// 1. Removing an override while callers still invoke the base method:
class Device final : public SkDevice {
-   sk_sp<SkSpecialImage> snapSpecialScaled(const SkIRect& subset,
-                                           const SkISize& dstDims) override;
};

// 2. Only removing dead override from one subclass, leaving dead base/subclass methods:
// src/gpu/ganesh/Device.h
-   sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
```

Good:

```cpp
// 1. Retain overrides when polymorphic callers still invoke the base method:
class Device final : public SkDevice {
    sk_sp<SkSpecialImage> snapSpecialScaled(const SkIRect& subset,
                                            const SkISize& dstDims) override;
};

// 2. When completely unused, delete across base interface and all subclasses:
// src/core/SkDevice.h: - virtual sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&);
// and delete overrides across Ganesh, Graphite, PDF, and Bitmap device subclasses.
```

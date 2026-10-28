---
name: skia/api/retain-public-convenience-apis-for-bindings
description: When refactoring public headers, do not deprecate or stage removal of public convenience methods with SK_LEGACY_* flags without verifying usage in bindings like CanvasKit
rubric: design
severity: should_fix
---

**Rule.** Do not deprecate or stage for removal existing public convenience
methods in public headers (e.g., using SK_LEGACY_* or SK_SUPPORT_LEGACY_* flags)
during refactorings without verifying whether language and Wasm bindings (such
as modules/canvaskit) rely on them. If a convenience method can cleanly execute
using existing public APIs without requiring private class internals, retain it
as a public member method rather than breaking bindings and forcing duplicate
downstream reimplementations.

**Why.** Public convenience methods on core classes are consumed not only by
external embedders like Android and Chromium, but also by internal language and
Wasm bindings such as CanvasKit. Deprecating or removing them forces bindings to
needlessly reimplement identical utility logic, whereas retaining them as public
convenience wrappers preserves backward compatibility at minimal maintenance
cost.

## Examples

Bad:

```cpp
// In include/core/SkPath.h:
// TODO: update Android/Chrome to use external helpers so we can remove this flag
#define SK_LEGACY_PATH_INTERPOLATE
#ifdef SK_LEGACY_PATH_INTERPOLATE
bool interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const;
#endif
```

Good:

```cpp
// In include/core/SkPath.h:
// Retain convenience method for bindings like CanvasKit even if implementation is external
bool interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const;
```

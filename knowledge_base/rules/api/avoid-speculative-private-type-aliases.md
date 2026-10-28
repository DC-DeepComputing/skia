---
name: skia/api/avoid-speculative-private-type-aliases
description: When modifying or replacing header symbols, retain backward-compatible aliases if downstream clients depend on them, but delete unreferenced APIs directly without speculative stubs
rubric: design
severity: should_fix
---

**Rule.** When refactoring, renaming, or replacing public macros, methods, or
types in headers (include/** and modules/**/include/**), verify downstream
client usage across embedders (Chromium, Android, Google3, Flutter). If external
clients depend on the symbol, retain backward-compatible aliases or forwarding
definitions (such as #define macros or using aliases) rather than abruptly
deleting them. Conversely, if downstream search confirms no external clients
reference the symbol, delete the API, obsolete forward declarations, and unused
types directly in the same change instead of introducing speculative
compatibility aliases, dummy stub methods, or deprecation TODO comments.

**Why.** Public headers define Skia's API surface consumed by external
embedders; deleting referenced symbols breaks downstream builds, so transitional
aliases are necessary when clients depend on them. However, introducing
speculative shims or stubs for unused symbols creates dead code and maintenance
debt when downstream code search confirms no callers exist.

## Examples

Bad:

```cpp
// Bad: Abruptly deleting a public macro or function still used by downstream embedders
// include/core/SkScalar.h
#define SkScalarHalf(a)             ((a) * SK_ScalarHalf)
// SkScalarAve removed outright without backward-compatible alias

// Bad: Speculative compatibility alias and tracking TODO for a private type
// include/private/SkEncodedInfo.h
// TODO(https://issues.skia.org/issues/12345): Remove once no dependencies use ICCProfile.
using ICCProfile = SkCodecs::ColorProfile;

// Bad: Leaving dummy deprecation stubs for unreferenced public APIs
// include/core/SkPath.h
struct SkArc;
// Deprecated -- will be removed
bool isArc(SkArc*) const { return false; }
```

Good:

```cpp
// Good: Retain backward-compatible alias or forwarder when downstream callers depend on it
// include/core/SkScalar.h
#define SkScalarHalf(a)             ((a) * SK_ScalarHalf)
#define SkScalarAve(a, b)           sk_float_midpoint(a, b)

// Good: Downstream code search confirms no external clients reference the symbol;
// delete the obsolete method, unused forward declaration, or old type alias directly
// without leaving transitional shims or deprecation stubs.
```

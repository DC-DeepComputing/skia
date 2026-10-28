---
name: skia/tests/prefer-generic-test-utilities
description: When writing or modifying test utilities and assertions, reuse generic helpers taking standard types like SkPixmap rather than introducing backend-specific duplicates
rubric: design
severity: should_fix
---

**Rule.** Do not introduce or maintain backend-specific duplicates of test
utility functions when a generic helper operating on standard types (such as
SkPixmap or SkBitmap) already exists or can be used. Verify whether existing
common utilities (such as CheckSolidPixels or ComparePixels) fulfill the
requirement before authoring backend-prefixed test helpers.

**Why.** Duplicating generic test utilities with backend-prefixed variants that
operate on standard data types adds maintenance overhead and code duplication
across backend test suites. Reusing generic utilities ensures consistent
validation logic and bug fixes across all backends.

## Examples

Bad:

```cpp
// Redundant backend-specific helper when generic CheckSolidPixels(..., const SkPixmap&, ...) exists
bool CheckGaneshSolidPixels(const SkColor4f& col,
                            const SkPixmap& pixmap,
                            const float tolRGBA[4],
                            std::function<ComparePixmapsErrorReporter>& error);
```

Good:

```cpp
// Use the generic SkPixmap-based test utility directly
bool CheckSolidPixels(const SkColor4f& col,
                      const SkPixmap& pixmap,
                      const float tolRGBA[4],
                      std::function<ComparePixmapsErrorReporter>& error);
```

---
name: skia/tests/assert-harness-setup-with-skassert
description: When allocating fundamental test harness fixtures or raster surfaces in unit tests, assert success directly with SkASSERT rather than pairing REPORTER_ASSERT with defensive early returns
rubric: readability
severity: should_fix
---

**Rule.** In unit tests, use SkASSERT rather than REPORTER_ASSERT paired with an
early return (such as if (!surface) return;) for fundamental harness setup
allocations like creating an in-memory SkSurface, canvas, or test fixture.
Reserve REPORTER_ASSERT for verifying operations and behavior under test.

**Why.** Fundamental test harness allocations succeed deterministically unless
the environment is in an unrecoverable state (such as out of memory). Combining
REPORTER_ASSERT with early returns adds redundant defensive boilerplate;
asserting directly with SkASSERT ensures fatal harness failures fail fast at
debug time without cluttering test control flow.

## Examples

Bad:

```cpp
sk_sp<SkSurface> surface = create_white_raster_surface();
REPORTER_ASSERT(reporter, surface);
if (!surface) {
    return;
}
```

Good:

```cpp
sk_sp<SkSurface> surface = create_white_raster_surface();
SkASSERT(surface);
```

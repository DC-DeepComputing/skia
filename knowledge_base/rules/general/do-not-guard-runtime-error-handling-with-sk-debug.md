---
name: skia/do-not-guard-runtime-error-handling-with-sk-debug
description: When implementing runtime checks, release assertions, or state tracking queried by production code, do not guard logic or calculations in SK_DEBUG blocks
rubric: correctness
severity: blocking
---

**Rule.** Do not place runtime validation, early returns, state tracking, or
dependent calculations inside #if defined(SK_DEBUG), #ifdef SK_DEBUG, or
SkDEBUGCODE(...) blocks when they are needed by production APIs, runtime control
flow, or release assertions (such as SkASSERT_RELEASE). Ensure both the checks
and any underlying state updates or calculations execute unconditionally across
debug and release builds so runtime behavior does not diverge.

**Why.** Code inside SK_DEBUG and SkDEBUGCODE(...) blocks is stripped from
release binaries. Guarding runtime checks or state tracking causes release
builds to bypass error handling, rely on stale or uninitialized variables in
SkASSERT_RELEASE, or return inaccurate state from production APIs.

## Examples

Bad:

```cpp
// Runtime error check stripped in release builds:
#if defined(SK_DEBUG)
    if (!texture->isInstantiated()) {
        SKGPU_LOG_W("Uninstantiated texture: %s", texture->label());
        return false;
    }
#endif

// Production API query or state tracking guarded by SK_DEBUG:
#ifdef SK_DEBUG
    if (!fCurrentCommandBuffer->hasWork()) {
        return false;
    }
#endif

// Variable needed by release assertion only computed in debug builds:
#if defined(SK_DEBUG)
    fDstRowBytes = static_cast<size_t>(frameWidth) * dstBytesPerPixel;
#endif
SkASSERT_RELEASE(dstRow.size() >= fDstRowBytes);
```

Good:

```cpp
// Runtime error check executes unconditionally in all builds:
if (!texture->isInstantiated()) {
    SKGPU_LOG_W("Uninstantiated texture: %s", texture->label());
    return false;
}

// Production API query evaluates state unconditionally:
if (!fCurrentCommandBuffer->hasWork()) {
    return false;
}

// Compute supporting calculations unconditionally for release assertions:
fDstRowBytes = static_cast<size_t>(frameWidth) * dstBytesPerPixel;
SkASSERT_RELEASE(dstRow.size() >= fDstRowBytes);
```

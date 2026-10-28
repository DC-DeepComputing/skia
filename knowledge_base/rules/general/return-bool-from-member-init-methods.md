---
name: skia/return-bool-from-member-init-methods
description: When implementing member initialization or validation helpers on ref-counted objects, return bool instead of sk_sp<T> to avoid unnecessary atomic ref-count churn from sk_ref_sp(this)
rubric: safety
severity: should_fix
---

**Rule.** When implementing member initialization, completion, or validation
methods on ref-counted classes, return bool rather than sk_sp<T> returning
sk_ref_sp(this). Let callers that already own an sk_sp<T> evaluate the boolean
result and conditionally retain or discard their existing smart pointer (e.g.,
init() ? sp : nullptr) without modifying reference counts.

**Why.** Returning sk_ref_sp(this) from an internal member helper forces
unnecessary atomic reference-count increments and decrements when the caller
already owns the instance. Returning a bool lets the caller retain or drop its
existing sk_sp directly without atomic ref-count churn.

## Examples

Bad:

```cpp
// Member method returns sk_sp<T>, incurring atomic ref-count churn:
sk_sp<SkPathData> SkPathData::finishInit(std::optional<SkRect> bounds) {
    if (!this->validate()) {
        return nullptr;
    }
    fBounds = bounds.value_or(this->computeBounds());
    return sk_ref_sp(this);
}

// Caller:
return path->finishInit(bounds);
```

Good:

```cpp
// Member method returns bool, avoiding atomic ref churn:
bool SkPathData::finishInit(std::optional<SkRect> bounds) {
    if (!this->validate()) {
        return false;
    }
    fBounds = bounds.value_or(this->computeBounds());
    return true;
}

// Caller retains or drops existing sk_sp without ref-count modifications:
return path->finishInit(bounds) ? path : nullptr;
```

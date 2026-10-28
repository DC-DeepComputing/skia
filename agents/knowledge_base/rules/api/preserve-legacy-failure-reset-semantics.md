---
name: skia/api/preserve-legacy-failure-reset-semantics
description: When reimplementing legacy mutating methods as wrappers over std::optional factories, preserve the legacy failure contract by resetting to default or identity state
rubric: correctness
severity: should_fix
---

**Rule.** When wrapping or reimplementing legacy mutating methods (such as bool
set...(...)) with modern factory functions returning std::optional<T>, verify
and preserve the legacy method's failure contract. If the legacy method
guaranteed resetting the instance to an identity or default state on failure,
call this->reset() before returning false rather than leaving the object in its
prior state.

**Why.** Callers relying on the legacy method may depend on the established
contract that the instance is cleared or reset to identity on failure. Returning
false without resetting leaves the instance in its prior state, causing
behavioral regressions across downstream consumers.

## Examples

Bad:

```cpp
bool setRectToRect(const SkRect& src, const SkRect& dst, ScaleToFit stf) {
    if (auto mx = Rect2Rect(src, dst, stf)) {
        *this = *mx;
        return true;
    }
    return false;
}
```

Good:

```cpp
bool setRectToRect(const SkRect& src, const SkRect& dst, ScaleToFit stf) {
    if (auto mx = Rect2Rect(src, dst, stf)) {
        *this = *mx;
        return true;
    }
    this->reset();
    return false;
}
```

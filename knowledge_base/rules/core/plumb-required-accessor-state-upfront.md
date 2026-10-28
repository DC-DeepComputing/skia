---
name: skia/core/plumb-required-accessor-state-upfront
description: When implementing accessor methods or query hooks on new or refactored types, plumb required state upfront instead of returning dummy placeholder values
rubric: correctness
severity: should_fix
---

**Rule.** Do not leave placeholder dummy return values (such as returning 0,
false, or nullptr with a TODO) on methods or accessors when callers or
optimization branches actively rely on that query. Plumb the required state
through the underlying structure and initialize it upfront so callers and
downstream optimizations function correctly.

**Why.** Returning placeholder values from query methods that are already
invoked by active code paths causes callers to malfunction, misclassify data, or
silently skip optimizations (such as line-only polygon paths). Storing and
initializing the necessary state upfront ensures callers receive accurate
metadata.

## Examples

Bad:

```cpp
// Bad: Returning a placeholder value causes callers relying on segment masks to miss optimizations
class SkPathRaw {
public:
    uint32_t getSegmentMasks() const { return 0; }  // TODO
};
```

Good:

```cpp
// Good: Plumb and store the actual state upfront so active callers query accurate data
class SkPathRaw {
public:
    uint32_t getSegmentMasks() const { return fSegmentMask; }
private:
    uint8_t fSegmentMask;
};
```

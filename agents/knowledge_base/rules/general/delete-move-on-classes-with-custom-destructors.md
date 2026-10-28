---
name: skia/delete-move-on-classes-with-custom-destructors
description: When a class manages raw resources in a custom destructor, do not default move constructors; delete move operations or construct in-place with std::in_place
rubric: safety
severity: should_fix
---

**Rule.** Do not default move constructors (= default) on classes that perform
manual resource cleanup or unrefing in their destructor. If move semantics
cannot safely transfer ownership, delete the move constructor (= delete) and
construct instances in-place inside std::optional using std::in_place or
.emplace().

**Why.** A defaulted move constructor performs shallow member-wise copying
without resetting raw pointers or cache references in the moved-from object.
When the moved-from temporary is destroyed, its destructor prematurely frees or
unrefs the underlying resource, leaving the moved-to instance with dangling
pointers or triggering double-free assertions.

## Examples

Bad:

```cpp
class NinePatch {
public:
    NinePatch(NinePatch&&) = default;  // Shallow copies raw resource pointers!
    ~NinePatch();  // Unrefs fCache or frees fMask.fImage
};

return NinePatch(mask, bounds, center, cache);
```

Good:

```cpp
class NinePatch {
public:
    NinePatch(NinePatch&&) = delete;  // Ownership transfer is not supported
    ~NinePatch();
};

return std::optional<SkMaskFilterBase::NinePatch>(
        std::in_place, mask, bounds, center, cache);
```

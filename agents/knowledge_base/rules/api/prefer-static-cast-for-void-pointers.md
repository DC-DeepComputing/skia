---
name: skia/api/prefer-static-cast-for-void-pointers
description: When converting a void* or const void* pointer to a concrete typed pointer, use static_cast instead of reinterpret_cast
rubric: readability
severity: should_fix
---

**Rule.** Use static_cast instead of reinterpret_cast when converting a void* or
const void* pointer to a concrete typed pointer (such as const uint8_t*).
Reserve reinterpret_cast strictly for conversions between unrelated non-void
pointer types or pointer-to-integer conversions.

**Why.** In standard C++, converting from void* to a typed object pointer is
well-defined and type-checked with static_cast. Using reinterpret_cast is overly
permissive, suppresses compiler safety checks, and misleadingly signals an
unsafe reinterpretation between unrelated types.

## Examples

Bad:

```cpp
// Bad: reinterpret_cast is overly permissive for void* conversions.
bool IsBmp(const void* buff, size_t bytesRead) {
    const rust::Slice<const uint8_t> slice{
        reinterpret_cast<const uint8_t*>(buff),
        bytesRead
    };
    return is_bmp(slice);
}
```

Good:

```cpp
// Good: static_cast correctly and safely converts from void*.
bool IsBmp(const void* buff, size_t bytesRead) {
    const rust::Slice<const uint8_t> slice{
        static_cast<const uint8_t*>(buff),
        bytesRead
    };
    return is_bmp(slice);
}
```

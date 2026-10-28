---
name: skia/assert-enum-sentinel-separation
description: When reserving integer sentinel values for invalid or unkeyed protocol states, verify with static_assert that valid enum or ID ranges exclude the sentinel
rubric: correctness
severity: should_fix
---

**Rule.** When reserving an integer sentinel value (such as 0, -1, or ~0) in
serialization, deserialization, or lookup protocols to represent an unkeyed,
inline, or invalid state, verify with a static_assert that valid enum or ID
ranges cannot collide with the sentinel value (e.g.,
static_assert(static_cast<uint32_t>(Enum::kStart) != 0)). Place the assertion
alongside the enum definition or protocol range validation logic.

**Why.** Serialization and lookup protocols often rely on a sentinel value (such
as 0 meaning 'not present' or 'fallback to inline payload') to distinguish
sentinels from valid table keys. If enum offsets or ID ranges are later
renumbered or extended to include the reserved sentinel, protocol decoders will
misinterpret valid entries as sentinels or corrupt data without compiler
warnings. A static_assert guarantees at compile time that the sentinel cannot
collide with valid enum ranges.

## Examples

Bad:

```cpp
// 0 is reserved in serialization to mean 'inline SkSL follows / not a stable key',
// but no compile-time check prevents StableKey::kStart from being 0:
enum class StableKey : uint32_t {
    kStart = 1,
    kBlend = kStart,
    // ...
    kLast = 50,
};

bool IsKnownRuntimeEffect(uint32_t candidate) {
    return candidate >= static_cast<uint32_t>(StableKey::kStart) &&
           candidate <= static_cast<uint32_t>(StableKey::kLast);
}
```

Good:

```cpp
// 0 is reserved in serialization to mean 'inline SkSL follows / not a stable key'
enum class StableKey : uint32_t {
    kStart = 1,
    kBlend = kStart,
    // ...
    kLast = 50,
};

// Guarantee at compile time that the valid range does not collide with sentinel 0
static_assert(static_cast<uint32_t>(StableKey::kStart) != 0);

bool IsKnownRuntimeEffect(uint32_t candidate) {
    return candidate >= static_cast<uint32_t>(StableKey::kStart) &&
           candidate <= static_cast<uint32_t>(StableKey::kLast);
}
```

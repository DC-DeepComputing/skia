---
name: skia/avoid-std-set-for-fixed-membership-checks
description: When checking membership in small fixed sets of values or enumerators, use a switch, if-condition, or constexpr array instead of std::set
rubric: safety
severity: should_fix
---

**Rule.** Do not use node-allocating or dynamic associative containers like
std::set or std::unordered_set to check membership in small, fixed sets of
values or enumerators. Use a switch statement, an if condition with logical OR,
or a flat constexpr std::array instead to eliminate runtime heap allocations.

**Why.** std::set is a node-based binary search tree whose elements each require
separate dynamic heap allocations on every function invocation, and
std::unordered_set requires bucket allocations and hash computations. For small,
fixed sets of constants or enums, a switch statement, if condition, or
compile-time constexpr array eliminates heap allocation overhead, improves
branch prediction, and reduces memory churn.

## Examples

Bad:

```cpp
// Bad: local std::set allocates tree nodes on the heap for each element per call
const std::set<SkColorType> kSupportedColorTypes = {
    kRGBA_8888_SkColorType,
    kRGBA_F16_SkColorType,
    kRGBA_1010102_SkColorType,
};
if (kSupportedColorTypes.find(dstColorType) == kSupportedColorTypes.end()) {
    return kUnimplemented;
}
```

Good:

```cpp
// Good: switch statement evaluates membership with zero heap allocations
switch (dstColorType) {
    case kRGBA_8888_SkColorType:
    case kRGBA_F16_SkColorType:
    case kRGBA_1010102_SkColorType:
        break;
    default:
        return kUnimplemented;
}
```

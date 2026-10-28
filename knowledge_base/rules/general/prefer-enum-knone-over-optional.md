---
name: skia/prefer-enum-knone-over-optional
description: When representing optional enum states or the absence of an action, define a kNone sentinel enumerator instead of wrapping the enum in std::optional<T>
rubric: readability
severity: should_fix
---

**Rule.** When an enum type can naturally represent the absence of a feature,
action, or requirement, define an explicit `kNone` sentinel enumerator within
the enum instead of wrapping the enum type in `std::optional<T>`. Use the
`kNone` enumerator directly for member variables, function parameters, and
return types.

**Why.** Defining a `kNone` sentinel directly in an enum avoids the wrapper
boilerplate, indirection, and padding overhead of `std::optional<T>`, while
simplifying member storage and call-site branching when 'none' is a valid domain
state.

## Examples

Bad:

```cpp
enum class BarrierType : uint8_t {
    kAdvancedNoncoherentBlend,
    kReadDstFromInput,
};

// Bad: Wrapping enum in std::optional adds padding overhead and unwrap checks.
std::optional<BarrierType> fBarrierBeforeDraws;
void recordDraw(std::optional<BarrierType> barrier);
```

Good:

```cpp
enum class BarrierType : uint8_t {
    kNone,
    kAdvancedNoncoherentBlend,
    kReadDstFromInput,
};

// Good: kNone sentinel simplifies storage, initialization, and comparisons.
BarrierType fBarrierBeforeDraws = BarrierType::kNone;
void recordDraw(BarrierType barrier);
```

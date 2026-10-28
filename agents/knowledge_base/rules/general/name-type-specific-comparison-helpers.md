---
name: skia/name-type-specific-comparison-helpers
description: When defining helper functions that compare concrete domain types or specialized containers, name them after the domain entities rather than generic container names like maps_are_equal
rubric: readability
severity: should_fix
---

**Rule.** When defining helper functions that compare concrete domain types or
specialized data structures, name the function after the specific domain
entities or comparison semantics being evaluated (such as
parameter_mappings_are_equal). Do not use generic collection names (such as
maps_are_equal or compare_lists) when the function does not support arbitrary
containers or implements specialized, domain-specific equality logic.

**Why.** Generic helper names like maps_are_equal or compare_lists falsely imply
that the function is a generic container utility supporting arbitrary element
types with standard equality semantics. Naming the helper after the concrete
domain entities clarifies the specific data structures and custom equality
criteria involved, making call sites self-documenting.

## Examples

Bad:

```cpp
// Generic container name falsely implies a general-purpose map comparison helper
static bool maps_are_equal(const SpecializedParameters& left, const SpecializedParameters& right) {
    if (left.count() != right.count()) {
        return false;
    }
    for (const auto& [key, leftExpr] : left) {
        const Expression** rightExpr = right.find(key);
        if (!rightExpr || !Analysis::IsSameExpressionTree(*leftExpr, **rightExpr)) {
            return false;
        }
    }
    return true;
}
```

Good:

```cpp
// Explicitly names the domain entity and specialized comparison semantics
static bool parameter_mappings_are_equal(const SpecializedParameters& left,
                                         const SpecializedParameters& right) {
    if (left.count() != right.count()) {
        return false;
    }
    for (const auto& [key, leftExpr] : left) {
        const Expression** rightExpr = right.find(key);
        if (!rightExpr || !Analysis::IsSameExpressionTree(*leftExpr, **rightExpr)) {
            return false;
        }
    }
    return true;
}
```

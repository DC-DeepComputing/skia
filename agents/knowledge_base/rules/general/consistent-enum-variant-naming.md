---
name: skia/consistent-enum-variant-naming
description: When defining enums, prefer scoped enum class, omit redundant type prefixes, and name variants with parallel, state-based, and symmetrically qualified phrasing
rubric: readability
severity: should_fix
---

**Rule.** Prefer scoped enumerations (enum class) over unscoped enums for new
types. In scoped enums, name enumerators with a leading 'k' (in C++) without
repeating the enum type name as a prefix or suffix (e.g., kYes, not
kIsAnimated_Yes). Name sibling variants using parallel phrasing across shared
dimensions, qualifying partitioned categories symmetrically (e.g.,
kPointTransparent alongside kDirectionalTransparent, not bare kTransparent).
Name variants after a feature's state or policy (e.g., ForceOff) rather than
assuming an alternative mechanism (ForceInterpreter) or using misleading terms
like Prefer.

**Why.** Scoped enums provide type safety and namespace isolation without
redundant name stutter. Consistent parallel phrasing and state-based naming
prevent caller confusion about fallback behavior and alternative mechanisms,
while symmetric qualification across partitioned dimensions prevents readers
from mistaking an unqualified variant for a global catch-all.

## Examples

Bad:

```cpp
// Unscoped enum repeating type name in enumerator suffixes:
enum IsAnimated {
    kIsAnimated_Yes,
    kIsAnimated_No,
    kIsAnimated_Unknown,
};

// Inconsistent phrasing, misleading 'Prefer', and repeating enum name:
pub enum AutoHintingControl {
    PreferAutoOverHintsForGlyf,
    ForceForGlyfAndCff,
    ForceInterpreter,
    AutoAsFallback,
}

// Unqualified variant alongside qualified dimensional siblings:
enum class OccluderType {
    kTransparent,
    kDirectionalTransparent,
};
```

Good:

```cpp
// Scoped enum class with concise k-prefixed enumerators:
enum class IsAnimated {
    kYes,
    kNo,
    kUnknown,
};

// Parallel phrasing, state-based naming, and concise variants:
pub enum AutoHintingControl {
    ForceForGlyf,
    ForceForGlyfAndCff,
    ForceOff,
    Fallback,
}

// Symmetrically qualified across partitioned dimensions:
enum class OccluderType {
    kPointTransparent,
    kDirectionalTransparent,
};
```

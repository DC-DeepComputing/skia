---
name: skia/document-enums-encoding-bitfield-offsets
description: When declaring enums whose values represent bit offsets or shifts into a packed bitfield, document the encoding scheme and target field
rubric: readability
severity: nit
---

**Rule.** When defining enums whose enumerators represent bit offsets, shift
amounts, or total bit capacities for a packed bitfield rather than simple
sequential identity discriminants, document the encoding layout and identify the
target bitfield member. Add an explanatory comment at the enum declaration
clarifying that the values encode bit positions and indicating how the packed
field is structured.

**Why.** Enum values that encode bit offsets (such as values stepped by field
widths or sentinel bit totals) do not follow standard zero-based sequential
discriminant conventions. Without documentation, maintainers may not realize the
enum is tightly coupled to a specific bitfield layout, risking accidental value
renumbering, reordering, or bit collision bugs.

## Examples

Bad:

```cpp
// Enum values represent bit offsets in fActions, but this dual role is undocumented:
enum ActionType {
    kDirectMask = 0,
    kPath       = 2,
    kDrawable   = 4,
    kSDFT       = 6,
    kMask       = 8,
    kTotalBits  = 10,
};
```

Good:

```cpp
// Encodes the 2-bit offset of each action type within the packed fActions bitfield.
enum ActionType {
    kDirectMask = 0,
    kPath       = 2,
    kDrawable   = 4,
    kSDFT       = 6,
    kMask       = 8,
    kTotalBits  = 10,
};
```

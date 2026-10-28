---
name: skia/prefer-named-struct-for-tables
description: When defining lookup tables or static arrays of structured data, declare a named struct rather than an anonymous C-style struct definition
rubric: readability
severity: should_fix
---

**Rule.** Prefer declaring a named struct over an anonymous struct (such as
`const struct { ... } table[]`) when defining lookup tables, static mapping
arrays, or tabular data. Define the named struct type first (e.g., `struct
TableEntry`), then declare the array using that named type.

**Why.** C-style anonymous structs obscure entry types, making them impossible
to reference in helper function signatures, templates, or loop bindings without
decltype. Defining an explicit named struct improves readability, clarifies
member semantics, and allows subsequent code to refer to the entry type cleanly.

## Examples

Bad:

```cpp
// Anonymous struct table obscures the entry type
const struct {
    CicpId cicp_id;
    SkColorSpacePrimaries sk_primaries;
} cicp_table[] = {
    { CicpId::kRec709, kRec709 },
};
```

Good:

```cpp
// Explicit named struct allows clean type references in signatures and loops
struct TableEntry {
    CicpId cicp_id;
    SkColorSpacePrimaries sk_primaries;
};
static const TableEntry cicp_table[] = {
    { CicpId::kRec709, kRec709 },
};
```

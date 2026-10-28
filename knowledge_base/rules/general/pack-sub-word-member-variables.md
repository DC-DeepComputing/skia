---
name: skia/pack-sub-word-member-variables
description: When declaring struct or class data members, order fields by decreasing alignment and size, packing sub-word fields adjacently after larger members to minimize padding
rubric: safety
severity: should_fix
---

**Rule.** When declaring member data in structs or classes, order fields by
decreasing alignment and size—placing stricter-alignment and larger types (such
as pointers, 64-bit values, and complex structs) before word-sized or float
types, and grouping sub-word fields (such as bool, uint8_t, uint16_t, or
single-byte enum classes) adjacently at the end rather than interleaving them.
Additionally, group immutable (const) members together where feasible to clearly
separate invariant state from mutable fields and avoid alignment gaps across
mutability sections.

**Why.** Compilers align struct and class data members to their natural
boundaries, inserting silent padding bytes between mismatched field sizes or
before higher-alignment members. Ordering fields by decreasing alignment and
packing smaller or sub-word fields adjacently minimizes internal padding holes,
reducing object memory footprint and cache overhead without altering class
semantics.

## Examples

Bad:

```cpp
struct LayerInfo {
    SkSize   fSize; // 4-byte alignment placed before 8-byte aligned SkString
    SkString fName;
    bool     fVisible;
    void*    fData; // Pointer after sub-word field adds internal padding
};
```

Good:

```cpp
struct LayerInfo {
    SkString fName; // Strictest alignment (pointer) first
    void*    fData;
    SkSize   fSize; // 4-byte aligned members follow
    bool     fVisible; // Sub-word fields packed at the end
};
```

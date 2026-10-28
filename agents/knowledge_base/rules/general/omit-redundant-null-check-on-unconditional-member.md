---
name: skia/omit-redundant-null-check-on-unconditional-member
description: When accessing member pointers or smart pointers guaranteed non-null by construction, dereference them directly instead of using redundant ternary null guards
rubric: readability
severity: nit
---

**Rule.** Do not perform redundant null checks, ternary guards (such as ptr ?
ptr->member : nullptr), or defensive if-statements on member pointers or smart
pointers that are unconditionally initialized in constructors and guaranteed
non-null for the object's lifetime. Dereference the member directly.

**Why.** Defensive null checks on unconditionally initialized members create
unreachable dead branches, clutter accessors, and mislead readers into believing
the member is optional or uninitialized.

## Examples

Bad:

```cpp
const skjson::ObjectValue* getSlotsRoot() const {
    return fSlotManager ? fSlotManager->fSlotsRoot : nullptr;
}
```

Good:

```cpp
// Dereference directly when the member pointer is unconditionally initialized:
const skjson::ObjectValue* getSlotsRoot() const {
    return fSlotManager->fSlotsRoot;
}
```

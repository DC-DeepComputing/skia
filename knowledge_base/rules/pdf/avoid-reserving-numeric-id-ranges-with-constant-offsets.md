---
name: skia/pdf/avoid-reserving-numeric-id-ranges-with-constant-offsets
description: When allocating shared numeric keys or IDs across multiple entity types, allocate dynamically from a unified container instead of reserving arbitrary constant base offsets
rubric: design
severity: should_fix
---

**Rule.** Do not partition shared numeric key or ID namespaces across multiple
entity types by reserving arbitrary constant base offsets (such as kFirst...Key
= 50000). Instead, allocate keys dynamically and sequentially from a unified
container (such as a dynamic array of variants) or an explicit allocator.

**Why.** Hardcoding arbitrary base offsets introduces artificial capacity
ceilings, risks silent collisions when entity counts exceed initial assumptions,
and fragments related data into parallel collections. Dynamic sequential
allocation from a unified table ensures safe growth without arbitrary limits.

## Examples

Bad:

```cpp
// Reserving ID ranges using arbitrary constant offsets
const int kFirstContentItemStructParentKey = 100000;
static constexpr int kFirstXObjectStructParentsKey = 50000;

unsigned allocateStructParentsKeyForXObject() {
    return kFirstXObjectStructParentsKey + fXObjectMcidMappings.size();
}
```

Good:

```cpp
// Allocating keys sequentially from a unified dynamic table
using ParentTreeEntry = std::variant<Item, Stream>;
skia_private::TArray<ParentTreeEntry> fParentTree;

if (structParentsKey < 0) {
    structParentsKey = fParentTree.size();
    fParentTree.push_back(Stream());
}
```

---
name: skia/api/avoid-mutating-state-in-getters
description: When defining accessor methods or extraction helpers, keep get-prefixed queries side-effect-free and idempotent, renaming mutating or allocating operations to create
rubric: readability
severity: should_fix
---

**Rule.** Do not name a method with a 'get...' prefix if it mutates object
state, generates new unique identifiers, or allocates state on each call. Keep
getter methods and value-extraction helpers as pure, idempotent, or memoized
queries (factoring mutations into explicit setters, binding routines, or
callers), and name non-idempotent or state-allocating operations with
'create...' (e.g., createMarkIdForNodeId) or 'createNew...' instead.

**Why.** In Skia conventions, a 'get...' prefix conveys an idempotent,
side-effect-free accessor where repeated invocations with the same argument
return existing or cached values without modifying state. Embedding mutations or
allocating new IDs inside a getter violates caller expectations, breaks
const-correctness, and causes subtle state corruption when callers assume
repeated queries are safe and idempotent.

## Examples

Bad:

```cpp
// Bad: Mutating object state inside an accessor/extraction helper
const skjson::StringValue* AnimatablePropertyContainer::getSlotID(const skjson::ObjectValue* jobj) {
    if (jobj && (*jobj)["sid"]) {
        fHasSlotID = true; // Side effect inside getter
        return (*jobj)["sid"];
    }
    return nullptr;
}

// Bad: 'get' prefix on non-idempotent method that generates new IDs and mutates state
int getMarkIdForNodeId(int nodeId);
int getParentTreeIdForNodeId(int nodeId);
```

Good:

```cpp
// Good: Pure extraction helper with mutation handled explicitly in caller
const skjson::StringValue* ParseSlotID(const skjson::ObjectValue* jobj);
if (const auto* sid = ParseSlotID(jprop)) {
    fHasSlotID = true;
    TrackSlot(sid);
}

// Good: Explicit 'create' prefix clarifies non-idempotent ID allocation
int createMarkIdForNodeId(int nodeId);
int createStructParentKeyForNodeId(int nodeId);
```

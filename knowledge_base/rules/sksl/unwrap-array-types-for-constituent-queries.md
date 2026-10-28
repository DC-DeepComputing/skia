---
name: skia/sksl/unwrap-array-types-for-constituent-queries
description: When inspecting or traversing SkSL types to identify constituent types like structs, check type.isArray() and unwrap via type.componentType()
rubric: correctness
severity: should_fix
---

**Rule.** When inspecting, collecting, or traversing SkSL types to identify
constituent types (such as structs or interface blocks), check type.isArray()
and unwrap via type.componentType(). Do not query type.isStruct() directly on
unpeeled types without unwrapping arrays, as arrays of structs report false for
isStruct().

**Why.** In SkSL's type system, Type::isArray() returns true for array types
while Type::isStruct() returns false, even when the element type is a struct.
Analysis passes, usage trackers, and backend codegen visitors that check
isStruct() without unwrapping array component types fail to collect or emit
definitions for structs declared as arrays.

## Examples

Bad:

```cpp
void visitType(const Type& t) {
    // Misses structs when t is an array of structs (e.g. MyStruct[3])
    if (t.isStruct()) {
        int& structCount = fUsage->fStructCounts[&t];
        structCount += fDelta;
        for (const Field& f : t.fields()) {
            this->visitType(*f.fType);
        }
    }
}
```

Good:

```cpp
void visitType(const Type& t) {
    if (t.isArray()) {
        this->visitType(t.componentType());
        return;
    }
    if (t.isStruct()) {
        int& structCount = fUsage->fStructCounts[&t];
        structCount += fDelta;
        for (const Field& f : t.fields()) {
            this->visitType(*f.fType);
        }
    }
}
```

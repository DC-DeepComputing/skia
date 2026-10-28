---
name: skia/sksl/use-sksl-type-kunsizedarray-sentinel
description: In SkSL IR and backend codegen, query Type::isUnsizedArray() instead of inspecting columns() > 0, and use Type::kUnsizedArray instead of -1
rubric: readability
severity: should_fix
---

**Rule.** When checking whether an SkSL array type is sized or unsized, query
type.isUnsizedArray() (or !type.isUnsizedArray()) rather than inspecting
type.columns() > 0 or comparing columns against sentinel values. When
representing, checking, or emitting raw array dimension values, use the
canonical sentinel Type::kUnsizedArray instead of literal -1.

**Why.** In SkSL, unsized arrays store Type::kUnsizedArray (-1) in their column
count as an internal representation detail. Calling .isUnsizedArray() conveys
semantic intent and isolates callers from representation details, while using
Type::kUnsizedArray avoids hardcoded magic numbers for raw dimension sizes.

## Examples

Bad:

```cpp
// Bad: Inspecting column count or using magic number -1 for unsized arrays
if (type->columns() > 0) {
    result = this->writeInstruction(SpvOpTypeArray, ...);
}

for (int s : sizes) {
    if (s != -1) {
        this->write(std::to_string(s));
    }
}
```

Good:

```cpp
// Good: Querying isUnsizedArray() on Type and using Type::kUnsizedArray sentinel
if (!type->isUnsizedArray()) {
    result = this->writeInstruction(SpvOpTypeArray, ...);
}

for (int s : sizes) {
    if (s != Type::kUnsizedArray) {
        this->write(std::to_string(s));
    }
}
```

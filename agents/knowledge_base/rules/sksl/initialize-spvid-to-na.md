---
name: skia/sksl/initialize-spvid-to-na
description: When declaring SpvId variables in SkSL SPIR-V codegen that are conditionally assigned across branches, defensively initialize them to the NA sentinel
rubric: readability
severity: should_fix
---

**Rule.** In SkSL SPIR-V code generation, defensively initialize local `SpvId`
variables to the sentinel `NA` upon declaration when their values are not
immediately assigned. Do not leave `SpvId` variables uninitialized before
branchy or conditional assignment blocks.

**Why.** `SpvId` is a primitive unsigned integer typedef that contains
indeterminate stack garbage if declared without an initializer. Initializing to
the `NA` sentinel `((SpvId)-1)` prevents uninitialized variable reads if control
flow branches diverge during future edits and ensures unassigned IDs fail fast
when verified.

## Examples

Bad:

```cpp
SpvId tmpVar;
SpvId tmpValueId = NA;
if (arg.type().isRef()) {
    tmpVar = this->nextId(&arg.type());
} else {
    tmpVar = this->nextId(nullptr);
}
```

Good:

```cpp
SpvId tmpVar = NA;
SpvId tmpValueId = NA;
if (arg.type().isRef()) {
    tmpVar = this->nextId(&arg.type());
} else {
    tmpVar = this->nextId(nullptr);
}
```

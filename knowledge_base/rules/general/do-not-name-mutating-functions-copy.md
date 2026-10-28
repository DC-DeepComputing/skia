---
name: skia/do-not-name-mutating-functions-copy
description: When defining functions that mutate, consume, or move resources out of source arguments, do not name them Copy; use move or conversion naming
rubric: readability
severity: nit
---

**Rule.** Do not name a function Copy (or with a copy prefix) if it mutates or
moves resources out of its source arguments. Use move or conversion terminology
(such as move or convert) or accept parameters by rvalue reference (&&) or by
value when an operation transfers ownership.

**Why.** A function named Copy communicates non-destructive duplication where
the source input is preserved intact. Draining resources via std::move or
mutating source arguments inside a function named Copy violates caller
expectations, obscures ownership transfer, and easily leads to use-after-move
defects.

## Examples

Bad:

```cpp
// Bad: Function named 'Copy' empties source fields via std::move
void SkPDFTagTree::Copy(SkPDF::StructureElementNode& node, SkPDFTagNode* dst) {
    dst->fAttributes = std::move(node.fAttributes.fAttrs);
    dst->fAttributeNodeIds = std::move(node.fAttributes.fNodeIds);
}
```

Good:

```cpp
// Good: Operation reflects destructive ownership transfer in name and semantics
void SkPDFStructTree::move(SkPDF::StructureElementNode& node, SkPDFStructElem* structElem) {
    structElem->fAttributes = std::move(node.fAttributes.fAttrs);
    structElem->fAttributeElemIds = std::move(node.fAttributes.fElemIds);
}
```

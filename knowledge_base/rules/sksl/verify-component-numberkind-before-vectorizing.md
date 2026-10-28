---
name: skia/sksl/verify-component-numberkind-before-vectorizing
description: When vectorizing mismatched binary expressions in SkSL backend codegen, verify that operand component number kinds match before synthesizing splats or operations
rubric: correctness
severity: should_fix
---

**Rule.** In SkSL backend code generators handling binary expressions with
mismatched operand types, verify that operands share the same
componentType().numberKind() before synthesizing splats or vector operations.
Bail out (e.g., return unsupported()) if the component numeric kinds differ.

**Why.** Code generators assume mismatched operands (such as scalar-vector
operations) share a compatible underlying component type when synthesizing
splats. If expressions reach code generation with differing component numeric
kinds (such as float and int) without prior IR coercion, attempting to construct
a splat to the opposing type produces invalid operations or miscompilation.

## Examples

Bad:

```cpp
if (!left.type().matches(right.type())) {
    if (left.type().isScalar() && right.type().isVector()) {
        ConstructorSplat leftAsVector(left.fPosition, right.type(), left.clone());
        return this->pushBinaryExpression(leftAsVector, op, right);
    }
}
```

Good:

```cpp
if (!left.type().matches(right.type())) {
    if (left.type().componentType().numberKind() != right.type().componentType().numberKind()) {
        return unsupported();
    }
    if (left.type().isScalar() && right.type().isVector()) {
        ConstructorSplat leftAsVector(left.fPosition, right.type(), left.clone());
        return this->pushBinaryExpression(leftAsVector, op, right);
    }
}
```

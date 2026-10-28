---
name: skia/sksl/document-divergent-intrinsic-lowering-semantics
description: When lowering overloaded intrinsics with divergent semantics or using type-named opcodes on untyped slots in codegen, document operation semantics and type safety
rubric: readability
severity: nit
---

**Rule.** When lowering or emitting backend code for overloaded intrinsics where
parameter types alter core operation semantics (such as mix performing a float
blend versus a boolean component-wise select), or when using backend
instructions whose names imply a type restriction across generic slots, document
the operation semantics and type safety in an inline comment.

**Why.** Overloaded intrinsics and backend instructions with type-specific names
(such as mix_n_ints operating on untyped 32-bit slots) can mislead readers into
assuming arithmetic interpolation or type mismatches. Clarifying whether an
operation blends or selects, and explaining that the instruction operates
bitwise regardless of payload type, prevents confusion and mistaken
refactorings.

## Examples

Bad:

```cpp
if (arg2.type().componentType().isBoolean()) {
    if (!this->pushExpression(arg2)) {
        return unsupported();
    }
    fBuilder.ternary_op(BuilderOp::mix_n_ints, arg0.type().slotCount());
    return true;
}
```

Good:

```cpp
if (arg2.type().componentType().isBoolean()) {
    if (!this->pushExpression(arg2)) {
        return unsupported();
    }
    // The `mix_int` op isn't doing a lerp; it uses the third argument to select values
    // from the first and second arguments. It's safe for use with any type in arguments
    // 0 and 1.
    fBuilder.ternary_op(BuilderOp::mix_n_ints, arg0.type().slotCount());
    return true;
}
```

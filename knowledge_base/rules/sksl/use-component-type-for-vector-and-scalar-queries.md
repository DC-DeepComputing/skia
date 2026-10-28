---
name: skia/sksl/use-component-type-for-vector-and-scalar-queries
description: When checking or asserting whether an SkSL type is numeric, float, or boolean across scalars and vectors, query type.componentType().is<Predicate>() instead of scalar-only predicates or !type.isVoid()
rubric: readability
severity: should_fix
---

**Rule.** In SkSL type queries, assertions, and IR validation, when checking
whether a type represents a numeric, float, integer, or boolean value across
both scalars and vectors, query type.componentType().is<Predicate>() (such as
type.componentType().isNumber() or type.componentType().isFloat()). Do not call
scalar-only predicates like type.isNumber() directly when vectors are permitted,
and do not fall back to overly loose negative checks like !type.isVoid().

**Why.** In SkSL's type system, Type::isNumber(), isFloat(), isInteger(), and
related predicates return true only for scalar types; compound types like
VectorType return kNonnumeric for numberKind(). Calling type.componentType()
unwraps both vectors and scalars to their underlying scalar component type,
enabling precise type validation while avoiding loose checks like !type.isVoid()
that unintentionally permit non-numeric types.

## Examples

Bad:

```cpp
// Loose check permits non-numeric types like booleans, matrices, or textures:
SkASSERT(!call.type().isVoid());

// Fails for vectors (VectorType::numberKind() is non-numeric):
if (type.isNumber()) {
    ...
}
```

Good:

```cpp
// Accurately validates numeric scalars and vectors:
SkASSERT(call.type().componentType().isNumber());

// Correctly unwraps scalars and vectors before querying element type:
if (type.componentType().isNumber()) {
    ...
}
```

---
name: skia/document-non-obvious-offsets-and-deltas
description: When hardcoding non-obvious numeric offsets, composite deltas, or stack-depth adjustments, document their mathematical derivation and component quantities in explanatory comments
rubric: readability
severity: nit
---

**Rule.** When returning or assigning hardcoded numeric literals that represent
composite offsets, deltas, or adjustments (such as net stack-depth changes or
buffer strides), document the derivation in an explanatory comment. Detail the
constituent arithmetic or component quantities (e.g., number of items consumed
versus produced) rather than leaving unexplained numeric literals.

**Why.** Hardcoded numeric literals representing net offsets or deltas look like
arbitrary magic numbers and are prone to calculation errors during maintenance.
Adding explanatory comments that detail what is added, consumed, or produced
makes the arithmetic verifiable at a glance and prevents accidental regressions
when operations evolve.

## Examples

Bad:

```cpp
case BuilderOp::dot_2_floats:
    return -3;
case BuilderOp::dot_3_floats:
    return -5;
case BuilderOp::dot_4_floats:
    return -7;
```

Good:

```cpp
case BuilderOp::dot_2_floats:
    return -3;  // consumes two 2-slot vectors and emits one scalar
case BuilderOp::dot_3_floats:
    return -5;  // consumes two 3-slot vectors and emits one scalar
case BuilderOp::dot_4_floats:
    return -7;  // consumes two 4-slot vectors and emits one scalar
```

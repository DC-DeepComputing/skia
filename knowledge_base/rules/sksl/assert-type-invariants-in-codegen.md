---
name: skia/sksl/assert-type-invariants-in-codegen
description: In SkSL backend codegen, assert type invariants and helper preconditions with SkASSERT instead of defensive fallback branches for forbidden types or scalar cases
rubric: readability
severity: should_fix
---

**Rule.** In SkSL backend code generators and IR analysis passes, do not add
defensive runtime fallback branches for types forbidden by the language
specification (such as matrices for min, max, or clamp), nor inside specialized
emission helpers (such as vector scalarizers) for non-specialized or scalar
cases. Assert type invariants and helper preconditions using SkASSERT (such as
SkASSERT(!type.isMatrix()) or SkASSERT(columns > 1)), query semantic type
predicates (like !type.isVector() or type.isScalar()), and route standard or
scalar cases to their emission helpers at the call site.

**Why.** Expressions entering backend code generation have already passed
front-end type validation, and specialized helpers rely on callers to route
appropriate constructs. Defensive branches and fallbacks introduce dead or
misleading code, obscure what types an intrinsic or helper truly handles, and
prevent debug builds from catching contract violations at call sites.

## Examples

Bad:

```cpp
// Caller dispatch:
int rows = c.type().rows();
int columns = c.type().columns();
if (fCaps.fVectorClampMinMaxSupport || columns == 1 || rows > 1) {
    this->writeSimpleIntrinsic(c);
} else {
    this->writeScalarizedIntrinsicCall(c);
}

// Specialized helper:
void MetalCodeGenerator::writeScalarizedIntrinsicCall(const FunctionCall& c) {
    if (c.arguments()[0]->type().columns() > 1) {
        // scalarize per component
    } else {
        this->writeSimpleIntrinsic(c);
    }
}
```

Good:

```cpp
// Caller dispatch:
SkASSERT(!c.type().isMatrix());
if (fCaps.fVectorClampMinMaxSupport || !c.type().isVector()) {
    this->writeSimpleIntrinsic(c);
} else {
    this->writeScalarizedIntrinsicCall(c);
}

// Specialized helper:
void MetalCodeGenerator::writeScalarizedIntrinsicCall(const FunctionCall& c) {
    SkASSERT(c.arguments()[0]->type().columns() > 1);
    // scalarize per component
}
```

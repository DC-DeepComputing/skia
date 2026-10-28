---
name: skia/sksl/compare-target-types-when-eliding-casts
description: When eliding type cast expressions in shader backend code generators, compare translated target types directly rather than surrogate property flags like precision
rubric: correctness
severity: should_fix
---

**Rule.** When determining whether a type cast or constructor expression can be
elided as a no-op in backend code generators, compare translated target types
directly (e.g., `to_wgsl_type(dest) == to_wgsl_type(src)`) rather than surrogate
property flags (such as `type_is_low_precision`). Do not skip emitting cast
expressions based on shared property equality when the underlying types may
still differ.

**Why.** Surrogate property flags only test a single dimension of a type (such
as precision level) and will evaluate to equal for distinct types that happen to
share that property (such as two different non-low-precision types like `int`
and `float`). Directly comparing translated target types ensures a cast is only
elided when the conversion is an identity operation in the target language,
preventing uncast expressions and target compilation errors.

## Examples

Bad:

```cpp
if (type_is_low_precision(ctor.type().componentType(), fContext) ==
    type_is_low_precision(ctor.argument()->type().componentType(), fContext)) {
    return this->assembleExpression(*ctor.argument(), parentPrecedence);
}
```

Good:

```cpp
if (to_wgsl_type(fContext, ctor.type().componentType()) ==
    to_wgsl_type(fContext, ctor.argument()->type().componentType())) {
    return this->assembleExpression(*ctor.argument(), parentPrecedence);
}
```

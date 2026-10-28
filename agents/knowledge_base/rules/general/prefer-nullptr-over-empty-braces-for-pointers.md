---
name: skia/prefer-nullptr-over-empty-braces-for-pointers
description: When passing null pointer arguments to functions expecting pointer parameters, pass explicit nullptr (e.g. /*param=*/nullptr) rather than empty braces {}
rubric: readability
severity: nit
---

**Rule.** Pass explicit nullptr (including inline parameter comments such as
/*paramName=*/nullptr) rather than empty braces {} when supplying a null pointer
argument to a function parameter expecting a pointer. Reserve {} and
/*paramName=*/{} for default-constructible value objects, structs, or empty
spans.

**Why.** While C++ allows empty braces {} to value-initialize raw pointers to
null, {} obscures parameter types by making pointer arguments look like
default-constructed structs, options, or containers. Passing nullptr conveys
pointer semantics unambiguously, clarifies caller intent, and avoids ambiguity
in overloaded functions.

## Examples

Bad:

```cpp
// Ambiguous whether parameters expect structs/options or raw pointers
fDC->recordDraw(renderer, localToDevice, Geometry(shape), clip, order,
                paintID, DstUsage::kNone,
                scopedDrawBuilder.gatherer(), /*stroke=*/{},
                /*latestDepthLayer=*/{});
```

Good:

```cpp
// Explicit nullptr clarifies that arguments are null pointers
fDC->recordDraw(renderer, localToDevice, Geometry(shape), clip, order,
                paintID, DstUsage::kNone,
                scopedDrawBuilder.gatherer(), /*stroke=*/nullptr,
                /*latestDepthLayer=*/nullptr);
```

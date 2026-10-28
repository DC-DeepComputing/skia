---
name: skia/collapse-nested-if-with-duplicate-fallback
description: When nested if statements have no intermediate code and no else branches, or share duplicate fallback blocks, collapse them using &&
rubric: readability
severity: nit
---

**Rule.** Flatten nested if statements into a single compound condition using &&
when an inner operation relies on an outer precondition check and there is no
intermediate code or else branch. Additionally, when nested if statements
execute identical fallback or reset logic across their else branches, collapse
the nested checks into a single compound condition using &&, hoisting
side-effect-free variable declarations so both conditions can be evaluated
together.

**Why.** Combining precondition guards and dependent operations into a single
condition using && reduces nesting and indentation while safely relying on
short-circuit evaluation. Collapsing checks that share identical fallback
branches eliminates duplicate fallback code and avoids maintenance divergence.

## Examples

Bad:

```cpp
// Precondition guard with no intermediate code or else branch:
if (metadata.fOrigin.has_value()) {
    if (!write_entry(kOriginTag, SkTiff::kTypeUnsignedShort, 1,
            metadata.fOrigin.value(), &endOfData, &stream, &buffer)) {
        return nullptr;
    }
}

// Duplicate fallback logic across nested branches:
const size_t prevStride = fInstanceStride ? fInstanceStride : fVertexStride;
if (prevStride) {
    const size_t currStride = instanceStride ? instanceStride : vertexStride;
    if (currStride) {
        fPendingBase = computeBase(prevStride, currStride);
    } else {
        fPendingBase = 0;
    }
} else {
    fPendingBase = 0;
}
```

Good:

```cpp
// Flattened into a single short-circuit condition:
if (metadata.fOrigin.has_value() &&
    !write_entry(kOriginTag, SkTiff::kTypeUnsignedShort, 1,
                 metadata.fOrigin.value(), &endOfData, &stream, &buffer)) {
    return nullptr;
}

// Hoisted declaration and collapsed into a single fallback branch:
const size_t prevStride = fInstanceStride ? fInstanceStride : fVertexStride;
const size_t currStride = instanceStride ? instanceStride : vertexStride;
if (prevStride && currStride) {
    fPendingBase = computeBase(prevStride, currStride);
} else {
    fPendingBase = 0;
}
```

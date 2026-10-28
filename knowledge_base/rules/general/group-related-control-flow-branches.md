---
name: skia/group-related-control-flow-branches
description: When structuring multi-branch control flow with deferred or special-case checks, group related conditions together rather than interleaving unrelated dispatch branches
rubric: readability
severity: nit
---

**Rule.** Group related special-case conditions and companion checks together in
multi-branch control-flow structures, particularly when an early guard defers
handling to a companion branch below. Do not interleave unrelated or orthogonal
checks between a deferred condition and its handling logic; consolidate
orthogonal dispatch strategies into separate, cohesive branch structures.

**Why.** Interleaving unrelated checks between an early condition and its
deferred handling logic obscures control flow and complicates reasoning about
branch invariants. Keeping related special cases adjacent and separating
orthogonal dispatch paths clarifies control flow and prevents accidental
fallthrough discrepancies.

## Examples

Bad:

```cpp
// Bad: Interleaving unrelated intrinsic checks between deferred sampler condition and its handler
if (paramIsSpecialized && !(param->type().isSampler() && fUseTextureSamplerPairs)) {
    return;
}
if (funcDecl.isIntrinsic() && !is_out(paramFlags)) {
    return this->writeExpression(arg, out);
} else if (arg.is<VariableReference>() && is_sampler_or_texture(arg)) {
    // Deferred sampler handling occurs here after unrelated check
    ...
}
```

Good:

```cpp
// Good: Group related special cases and deferred checks together
if (paramIsSpecialized && !(param->type().isSampler() && fUseTextureSamplerPairs)) {
    return;
} else if (arg.is<VariableReference>() && is_sampler_or_texture(arg)) {
    ...
}
// Consolidate orthogonal argument-passing dispatch into a cohesive chain
if (is_out(paramFlags)) {
    ...
} else if (funcDecl.isIntrinsic()) {
    return this->writeExpression(arg, out);
} else {
    ...
}
```

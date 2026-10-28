---
name: skia/prefer-predicate-over-default-instance-comparison
description: When testing whether an object possesses a specific state or property, call dedicated member predicates rather than comparing against a default-constructed instance
rubric: readability
severity: nit
---

**Rule.** When checking whether an object or descriptor exhibits a specific
condition, validity, or property, call dedicated member predicate methods (such
as isImmutable(), isValid(), or isEmpty()) rather than comparing equality or
inequality against a default-constructed instance (such as obj != Type() or obj
== Type()).

**Why.** Comparing against a default-constructed temporary is indirect, obscures
which specific field or state invariant is being validated, and risks subtle
bugs if default member initializers change. Dedicated predicate methods make
code self-documenting and avoid constructing redundant temporary objects.

## Examples

Bad:

```cpp
if (samplerDesc != SamplerDesc()) {
    sk_sp<Sampler> immutableSampler =
            resourceProvider->findOrCreateCompatibleSampler(samplerDesc);
    // ...
}
```

Good:

```cpp
if (samplerDesc.isImmutable()) {
    sk_sp<Sampler> immutableSampler =
            resourceProvider->findOrCreateCompatibleSampler(samplerDesc);
    // ...
}
```

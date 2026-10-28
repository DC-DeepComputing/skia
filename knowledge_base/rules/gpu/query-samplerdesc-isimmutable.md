---
name: skia/gpu/query-samplerdesc-isimmutable
description: When checking whether a Graphite SamplerDesc is immutable or dynamic, call samplerDesc.isImmutable() instead of comparing against a default-constructed SamplerDesc()
rubric: readability
severity: nit
---

**Rule.** In Graphite, query samplerDesc.isImmutable() (or
!samplerDesc.isImmutable()) to determine whether a sampler is immutable or
dynamic, rather than comparing against a default-constructed SamplerDesc() (such
as desc == SamplerDesc() or desc != SamplerDesc()).

**Why.** A default-constructed SamplerDesc has all bitfields (including filter
and tile modes) set to zero, so equality comparison against SamplerDesc() checks
whether all sampling options are zero rather than whether the sampler is
immutable. Calling isImmutable() directly checks the dedicated bitfield,
expressing intent clearly and correctly identifying dynamic samplers regardless
of other sampling parameters.

## Examples

Bad:

```cpp
if (numTexturesAndSamplers == 2 &&
    (!samplerDescArrPtr || samplerDescArr.at(0) == SamplerDesc())) {
    // Assuming sampler is dynamic
}
```

Good:

```cpp
if (numTexturesAndSamplers == 2 &&
    (!samplerDescArrPtr || !samplerDescArrPtr->at(0).isImmutable())) {
    // Correctly check whether sampler is dynamic
}
```

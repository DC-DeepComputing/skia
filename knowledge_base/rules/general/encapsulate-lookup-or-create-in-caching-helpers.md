---
name: skia/encapsulate-lookup-or-create-in-caching-helpers
description: When implementing helpers that synthesize or create cached objects, encapsulate find-or-create lookup and reuse inside the helper rather than delegating cache checks to callers
rubric: design
severity: should_fix
---

**Rule.** Encapsulate find-or-create and reuse logic inside caching and
synthesis helper methods rather than delegating cache lookups to callers.
Callers should unconditionally invoke the helper, and the helper should check
its internal cache, return existing cached entries if present, or create, cache,
and return the new entry.

**Why.** Delegating cache checks to callers duplicates lookup boilerplate across
call sites and risks state corruption or map clobbering if any caller calls the
helper directly without checking the cache first. Consolidating lookup and
insertion inside the helper guarantees cache invariants, prevents redundant
synthesis, and keeps call sites clean.

## Examples

Bad:

```cpp
// Caller inspects internal cache before invoking synthesis helper:
auto* cachedSampler = fSynthesizedSamplerMap.find(parameter);
auto [texture, sampler] =
        cachedSampler ? std::make_tuple((*cachedSampler)->fTexture.get(),
                                        (*cachedSampler)->fSampler.get())
                      : this->synthesizeTextureAndSampler(*parameter);
```

Good:

```cpp
// Caller unconditionally delegates to helper:
auto [texture, sampler] = this->synthesizeTextureAndSampler(*parameter);

// Synthesis helper checks internal cache directly:
std::tuple<const Variable*, const Variable*> SPIRVCodeGenerator::synthesizeTextureAndSampler(
        const Variable& combinedSampler) {
    if (auto* existing = fSynthesizedSamplerMap.find(&combinedSampler)) {
        return {(*existing)->fTexture.get(), (*existing)->fSampler.get()};
    }
    // ... synthesize, insert into fSynthesizedSamplerMap, and return ...
}
```

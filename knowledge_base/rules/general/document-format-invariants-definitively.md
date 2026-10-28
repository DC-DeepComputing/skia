---
name: skia/document-format-invariants-definitively
description: When documenting data formats, protocols, or encoding invariants in comments, state value representations definitively rather than using ambiguous modal verbs like 'may' or 'can'
rubric: readability
severity: nit
---

**Rule.** When documenting data layouts, serialization protocols, or encoding
schemes in code comments, state invariants and value representations
definitively (e.g., describing which value indicates a given mode or state).
Avoid permissive or hedging modal verbs like 'may' or 'can' that obscure whether
a representation is an established protocol guarantee or an optional behavior.

**Why.** Hedging modal verbs like 'may' or 'can' leave readers and reviewers
uncertain whether an encoding or sentinel value is a mandatory format invariant
or merely optional or speculative. Stating data format rules and sentinel
meanings directly ensures downstream decoders and maintainers can clearly
understand and rely on the established contract.

## Examples

Bad:

```cpp
// For any shaders that may use a combination of dynamic + immutable samplers, a value of
// "0" may be inserted as the first value of a SamplerDesc to indicate the use of a dynamic
// sampler. This enables the data to convey & maintain sampler binding order.
if (samplerData[i] == 0) {
    outDescs.push_back({});
    i++;
    continue;
}
```

Good:

```cpp
// Create a default-initialized SamplerDesc (one uint32). For dynamic samplers,
// this sentinel is inserted directly into outDescs. Otherwise, populate it
// with immutable sampler data before inserting.
SamplerDesc desc{};
size_t samplerDescLength = 1;
SkASSERT(desc.asSpan().size() == samplerDescLength);
```

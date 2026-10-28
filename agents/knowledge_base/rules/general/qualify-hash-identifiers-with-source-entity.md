---
name: skia/qualify-hash-identifiers-with-source-entity
description: When declaring hash parameters or member fields in composite types or functions, qualify identifiers with the source entity rather than generic hash or fHash
rubric: readability
severity: should_fix
---

**Rule.** When declaring hash parameters, local variables, or member fields in
composite classes, functions, or metadata structs, qualify the identifier with
the source entity (e.g., uniqueKeyHash or fUniqueKeyHash) rather than using
generic names like hash or fHash. Reserve bare fHash or hash() for standalone
key, token, or entry types that represent the hashable entity itself.

**Why.** In composite types, metadata structs, or functions operating across
multiple hashable structures, generic names like hash or fHash are ambiguous
about which entity was hashed. Explicitly naming the source entity clarifies
data flow and avoids naming collisions or misinterpretations when multiple
hashes are stored.

## Examples

Bad:

```cpp
struct PipelineInfo {
    PipelineInfo(const ShaderInfo& shaderInfo,
                 uint32_t hash,
                 uint32_t compilationID);

    const uint32_t fHash = 0;
};
```

Good:

```cpp
struct PipelineInfo {
    PipelineInfo(const ShaderInfo& shaderInfo,
                 uint32_t uniqueKeyHash,
                 uint32_t compilationID);

    const uint32_t fUniqueKeyHash = 0;
};
```

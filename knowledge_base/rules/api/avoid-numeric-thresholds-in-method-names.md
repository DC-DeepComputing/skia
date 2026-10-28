---
name: skia/api/avoid-numeric-thresholds-in-method-names
description: When declaring public API query methods, name methods after high-level semantic concepts rather than baking specific numeric thresholds into identifiers
rubric: readability
severity: nit
---

**Rule.** When declaring public API methods (such as format or capability
queries), prefer conventional, semantic names over identifiers that bake in
specific numeric thresholds or comparison criteria (e.g., use
hasHighBitDepthEncodedData instead of wasEncodedWith16bitsOrMorePerComponent).
Document the exact numerical threshold criteria and boundaries in header
documentation comments.

**Why.** Encoding exact numeric thresholds in method names creates awkward,
brittle identifiers that age poorly if criteria or format specs evolve. Semantic
names aligned with client expectations keep public interfaces durable and call
sites clean, while doc comments allow threshold nuances to be explained without
breaking API contracts.

## Examples

Bad:

```cpp
class SK_API SkCodec {
public:
    // Avoid baking specific numeric thresholds into public method names
    bool wasEncodedWith16bitsOrMorePerComponent() const;
};
```

Good:

```cpp
class SK_API SkCodec {
public:
    /**
     * Whether the encoded input uses high bit depth (16 or more bits per component).
     */
    bool hasHighBitDepthEncodedData() const;
};
```

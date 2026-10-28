---
name: skia/gpu/dynamic-sampler-data-count-is-zero
description: When inspecting or deserializing sampler descriptor data in Graphite's PaintParamsKey, identify dynamic samplers by checking for zero data indices rather than one
rubric: correctness
severity: nit
---

**Rule.** When inspecting, dumping, or deserializing sampler descriptor data in
Graphite (`PaintParamsKey`), verify that dynamic samplers are identified by
`dataIndexCount == 0` (or `dataLength == 0`). Do not expect dynamic samplers to
store one data index; dynamic samplers pass an empty span (`{}`) to `addData` to
record sampler binding order without embedding descriptor payload.

**Why.** In Graphite, immutable samplers embed non-zero data indices to store
`SamplerDesc` information, whereas dynamic samplers record an empty payload of
zero data indices. Checking for `dataIndexCount == 1` causes dynamic samplers to
be misidentified or rejected, corrupting string dumps or key deserialization.

## Examples

Bad:

```cpp
// Incorrectly assumes dynamic samplers store 1 index of data
if (dataIndexCount == 1) {
    str->append("(0)");
}
```

Good:

```cpp
// Dynamic samplers pass an empty data span, resulting in 0 data indices
if (dataIndexCount == 0) {
    str->append("(0)");
}
```

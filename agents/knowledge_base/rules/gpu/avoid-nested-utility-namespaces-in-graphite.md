---
name: skia/gpu/avoid-nested-utility-namespaces-in-graphite
description: When defining helper functions in Graphite utility headers, place free functions directly in skgpu::graphite with descriptive names instead of nested utility namespaces
rubric: readability
severity: nit
---

**Rule.** When declaring helper or conversion functions in Graphite utility
headers, place free functions directly in skgpu::graphite using descriptive,
entity-qualified names (e.g., SerializePipelineDesc or PipelineDescToData). Do
not introduce redundant nested utility namespaces (such as namespace
SerializationUtils).

**Why.** Graphite scopes all subsystem symbols directly under skgpu::graphite to
keep namespace hierarchy shallow and uniform. Unlike legacy Ganesh code that
defined global Gr*Utils namespaces, Graphite avoids extra utility namespace
levels in favor of explicit function identifiers at subsystem scope.

## Examples

Bad:

```cpp
namespace skgpu::graphite {
namespace SerializationUtils {

[[nodiscard]] bool Serialize(ShaderCodeDictionary*,
                             SkWStream*,
                             const GraphicsPipelineDesc&,
                             const RenderPassDesc&);
[[nodiscard]] bool Deserialize(const Caps*,
                               ShaderCodeDictionary*,
                               SkStream*,
                               GraphicsPipelineDesc*,
                               RenderPassDesc*);

}  // namespace SerializationUtils
}  // namespace skgpu::graphite
```

Good:

```cpp
namespace skgpu::graphite {

[[nodiscard]] bool SerializePipelineDesc(ShaderCodeDictionary*,
                                         SkWStream*,
                                         const GraphicsPipelineDesc&,
                                         const RenderPassDesc&);
[[nodiscard]] bool DeserializePipelineDesc(const Caps*,
                                           ShaderCodeDictionary*,
                                           SkStream*,
                                           GraphicsPipelineDesc*,
                                           RenderPassDesc*);

}  // namespace skgpu::graphite
```

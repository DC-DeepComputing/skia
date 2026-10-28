---
name: skia/api/name-composite-types-after-combined-roles
description: When naming composite types or type aliases bundling multiple components, name them after combined roles rather than generic *Helpers suffixes
rubric: readability
severity: should_fix
---

**Rule.** Name composite types, structs, and type aliases after the specific
roles, capabilities, or data components they bundle together (e.g.,
KeyAndDataBuilder). Do not use vague, generic catch-all suffixes like *Helper or
*Helpers (such as KeyContextHelpers) that obscure the bundled responsibilities.

**Why.** Generic suffixes like *Helper or *Helpers obscure the concrete
responsibilities and types grouped into an abstraction. Descriptively naming
composite types after their constituent roles makes their purpose
self-documenting at declarations, member access sites, and storage containers.

## Examples

Bad:

```cpp
// Vague catch-all *Helpers suffix obscures the two bundled roles
using KeyContextHelpers = std::pair<PipelineDataGatherer, PaintParamsKeyBuilder>;
```

Good:

```cpp
// Explicitly named after the combined data gathering and key building roles
using KeyAndDataBuilder = std::pair<PipelineDataGatherer, PaintParamsKeyBuilder>;
```

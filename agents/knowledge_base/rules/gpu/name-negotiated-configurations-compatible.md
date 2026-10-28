---
name: skia/gpu/name-negotiated-configurations-compatible
description: When declaring GPU methods that dynamically negotiate a configuration matching an input resource or format, name them getCompatible* rather than getDefault*
rubric: readability
severity: should_fix
---

**Rule.** Do not name methods getDefault* when they dynamically inspect an input
resource, format, or target descriptor to negotiate or select a supported
configuration; use getCompatible* (e.g., getCompatibleMSAASampleCount) or a more
descriptive query name instead. Reserve getDefault* for fixed, canonical
fallback values that do not depend on matching an input target.

**Why.** 'Default' implies a fixed, canonical fallback value independent of
target constraints. When a method evaluates input parameters (such as a
TextureInfo's format and capabilities) and computes or negotiates the best
supported configuration, 'getCompatible*' accurately conveys that the returned
value is dynamically matched to the input resource.

## Examples

Bad:

```cpp
// Misleading: implies a static default independent of the input texture
SampleCount Caps::getDefaultMSAASampleCount(const TextureInfo& info) const;
```

Good:

```cpp
// Accurate: dynamically evaluates info to find a compatible sample count
SampleCount Caps::getCompatibleMSAASampleCount(const TextureInfo& info) const;
```

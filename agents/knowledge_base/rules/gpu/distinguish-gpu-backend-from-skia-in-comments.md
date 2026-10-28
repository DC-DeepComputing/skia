---
name: skia/gpu/distinguish-gpu-backend-from-skia-in-comments
description: When documenting engine-specific capabilities, features, or requirements in GPU backend comments, refer specifically to Graphite or Ganesh rather than generic Skia
rubric: readability
severity: nit
---

**Rule.** In comments within GPU backends (such as Graphite or Ganesh), refer
specifically to the subsystem or engine (e.g., 'Graphite' or 'Ganesh') rather
than generic 'Skia' when describing requirements, capabilities, or features
specific to that engine. Keep comments precise to the active architecture rather
than implying project-wide Skia requirements.

**Why.** Skia contains multiple distinct GPU rendering engines and backends that
maintain separate capability checks, feature chains, and driver requirements.
Referring to generic 'Skia' in engine-specific code is imprecise and misleading
because other backends or core modules may handle features differently or not
require them at all.

## Examples

Bad:

```cpp
// Walk the feature chain once and extract any enabled features that Skia cares about.
```

Good:

```cpp
// Walk the feature chain once and extract any enabled features that Graphite cares about.
```

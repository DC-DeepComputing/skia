---
name: skia/gpu/identify-depth-only-draws-by-paint-id-validity
description: When classifying depth-only draws in Graphite draw recording, check for invalid paintID (!paintID.isValid()) rather than inspecting out-parameter nullness
rubric: readability
severity: should_fix
---

**Rule.** When identifying depth-only or non-shading draws (such as clips)
during Graphite draw recording, check whether the paint ID is invalid
(!paintID.isValid() or paintID == UniquePaintParamsID::Invalid()). Do not infer
whether a draw is depth-only based on the presence, nullness, or capturing of
out-parameters.

**Why.** In Graphite, depth-only draws do not execute shading pipelines and are
canonically represented by UniquePaintParamsID::Invalid(). Relying on paint ID
validity preserves established domain invariants and decouples draw-type
classification from parameter-capturing out-pointers or caller-specific output
requests.

## Examples

Bad:

```cpp
// Inferring depth-only status from whether an out-parameter was passed
if (captureParams != nullptr) {
    this->recordDepthOnly(renderer, geometry, clip, ordering);
}
```

Good:

```cpp
// Checking canonical paint ID validity to detect depth-only draws
if (!paintID.isValid()) {
    this->recordDepthOnly(renderer, geometry, clip, ordering);
}
```

---
name: skia/document-legacy-compatibility-checks
description: When retaining legacy-compatibility checks or using deprecated constructs for baseline target compatibility, document the constraints, target requirements, and cleanup milestones
rubric: readability
severity: nit
---

**Rule.** When retaining non-obvious or indirect checks to accommodate
deprecated or legacy features, document why direct checks cannot yet be used and
under what condition or milestone the check can be simplified (e.g., with a
cleanup TODO). Additionally, when deliberately using a deprecated specification
approach, legacy format, or older API pattern instead of modern standards to
support baseline target environments, document the rationale explaining the
target requirements and constraints that necessitate the choice.

**Why.** Omitting explanation for legacy compatibility workarounds or deprecated
specification constructs prompts reviewers and maintainers to question why
modern or direct alternatives are avoided, risking premature simplifications or
modernizations that break compatibility. Explaining target requirements,
constraints, and cleanup milestones clarifies deliberate choices and marks code
for future simplification once legacy support is retired.

## Examples

Bad:

```cpp
// Retaining indirect check without explaining legacy constraint:
if (SkStrokeRec(paint).getStyle() != SkStrokeRec::kFill_Style) {
    return nullptr;
}

// Stating an approach is deprecated without explaining baseline target requirements:
// In SPIR-V 1.3+, storage buffers use "StorageBuffer" and "Block". Here we implement
// the deprecated pre-1.3 approach with "Uniform" and "BufferBlock".
```

Good:

```cpp
// TODO: Once stroke-and-fill goes away, we can check the paint's style directly.
if (SkStrokeRec(paint).getStyle() != SkStrokeRec::kFill_Style) {
    return nullptr;
}

// In SPIR-V 1.3+, storage buffers can be declared with "StorageBuffer" and "Block",
// and pre-1.3 ("Uniform" and "BufferBlock") is deprecated. Since we target SPIR-V 1.0,
// we use the deprecated approach which is supported across baseline Vulkan environments.
```

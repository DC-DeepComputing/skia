---
name: skia/gpu/gpu-qualify-scope-specific-base-constants
description: When defining limits, thresholds, or constants in shared GPU base classes, qualify names to reflect specific subclass categories or execution paths
rubric: readability
severity: nit
---

**Rule.** When defining constants, capacity limits, or complexity thresholds in
shared base classes or interfaces, qualify the identifier with the specific
subclass category, backend, or execution path it governs (e.g.,
kMaxGPUPathRendererVerbs rather than kMaxPathVerbs). Do not use overly broad or
unqualified names for limits that apply only to a subset of implementations or
allow fallback to alternate paths.

**Why.** In class hierarchies where base classes coordinate multiple execution
strategies (such as hardware-accelerated GPU rendering and software fallback),
overly generic constant names falsely imply a global constraint across all
paths. Explicit domain qualification clarifies which implementations enforce the
threshold and prevents confusion about whether non-qualifying paths are subject
to the limit.

## Examples

Bad:

```cpp
class PathRenderer : public SkRefCnt {
public:
    // Bad: Generic name suggests a universal limit across all paths and renderers.
    static constexpr int kMaxPathVerbs = 1 << 14;
};
```

Good:

```cpp
class PathRenderer : public SkRefCnt {
public:
    // Good: Explicit qualification clarifies the limit applies specifically to GPU renderers.
    static constexpr int kMaxGPUPathRendererVerbs = 1 << 14;
};
```

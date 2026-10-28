---
name: skia/gpu/avoid-by-value-transforms-in-draw-records
description: When defining per-draw records like DrawParams in GPU recording, store large shared structures like Transform by const reference rather than by value
rubric: safety
severity: should_fix
---

**Rule.** In GPU draw recording pipelines, do not store large, frequently shared
structures like Transform (~144 bytes) by value in per-draw records (such as
DrawParams). Hold a const reference to deduplicated instances managed by
enclosing draw lists or arena storage (such as DrawListBase's SkTBlockList)
instead.

**Why.** Transform contains SkM44 matrices and metadata (~144 bytes) that are
frequently identical across consecutive draw commands. Storing Transform by
value in per-draw parameter structs substantially inflates per-draw memory
footprint, whereas referencing deduplicated instances avoids redundant data
copies across batches.

## Examples

Bad:

```cpp
class DrawParams {
    ...
private:
    Transform fTransform;
    Geometry  fGeometry;
};
```

Good:

```cpp
class DrawParams {
    ...
private:
    const Transform& fTransform;
    Geometry         fGeometry;
};
```

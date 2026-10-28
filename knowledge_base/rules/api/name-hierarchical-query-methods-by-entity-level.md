---
name: skia/api/name-hierarchical-query-methods-by-entity-level
description: When declaring multi-level hierarchical query or traversal APIs, name each method after the entity level and index parameter rather than specific subcases
rubric: readability
severity: should_fix
---

**Rule.** In multi-level hierarchical query or traversal APIs, name each query
method consistently after the structural entity level being inspected (matching
the index parameter and preceding level's count) rather than after a specific
subcase or return property. When traversing nested hierarchies, align method
names and index parameters symmetrically across levels (e.g., scanFace(...,
faceIndex, &numInstances) rather than scanDefaultInstance).

**Why.** Naming an intermediate hierarchical query after a specific subcase
(such as default instance) obscures the structural symmetry of the API and
misleads callers about what level of the hierarchy is being examined.
Symmetrical naming aligned with index arguments and child counts makes
navigation across hierarchy levels intuitive and discoverable.

## Examples

Bad:

```cpp
// Bad: Naming an intermediate hierarchy step after a subcase breaks structural symmetry:
class SkFontScanner {
public:
    virtual bool scanFile(SkStreamAsset* stream, int* numFaces) const = 0;
    virtual bool scanDefaultInstance(SkStreamAsset* stream, int faceIndex, int* numInstances) const = 0;
    virtual bool scanInstance(SkStreamAsset* stream, int faceIndex, int instanceIndex, ...) const = 0;
};
```

Good:

```cpp
// Good: Symmetrically naming each query method after the entity level and index parameter:
class SkFontScanner {
public:
    virtual bool scanFile(SkStreamAsset* stream, int* numFaces) const = 0;
    virtual bool scanFace(SkStreamAsset* stream, int faceIndex, int* numInstances) const = 0;
    virtual bool scanInstance(SkStreamAsset* stream, int faceIndex, int instanceIndex, ...) const = 0;
};
```

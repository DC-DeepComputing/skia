---
name: skia/api/document-object-semantics-on-definitions-not-setters
description: When documenting setter or attachment methods accepting polymorphic objects or dependencies, place behavioral contracts on the object's definition or factory rather than the setter
rubric: readability
severity: nit
---

**Rule.** Do not document an object's requirements, behavioral contracts, or
evaluation semantics on consumer setter or attachment methods that merely accept
or bind the object. Instead, document object contracts on the class interface,
header overview, or factory functions where the object type is defined and
created.

**Why.** Consumer setter methods are responsible for binding an object
reference, not specifying the object's intrinsic behavior or implementation
rules. Placing dependency requirements on consumer methods clutters the setter
API and hides critical contracts from developers implementing, subclassing, or
instantiating the object elsewhere.

## Examples

Bad:

```cpp
/** Sets SkShader to shader, decreasing SkRefCnt of the previous SkShader.
    Increments shader SkRefCnt by one. Note that the shader should return
    color that is premultiplied.
*/
void setShader(sk_sp<SkShader> shader);
```

Good:

```cpp
/** Sets SkShader to shader, decreasing SkRefCnt of the previous SkShader.
    Increments shader SkRefCnt by one.
*/
void setShader(sk_sp<SkShader> shader);

// In include/core/SkShader.h (on class or factory declaration):
/** Shaders specify the source color(s) for what is being drawn.
    Returned colors must be premultiplied.
*/
```

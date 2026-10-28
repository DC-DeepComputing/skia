---
name: skia/gpu/gpu-qualify-backend-specific-methods-in-shared-headers
description: When declaring member functions in shared headers implemented in backend-specific translation units, qualify method names with the backend rather than overloading
rubric: readability
severity: should_fix
---

**Rule.** When declaring member functions in shared headers whose definitions
reside in separate, backend-specific translation units (such as Ganesh vs.
Graphite), qualify each method name explicitly with its target backend (e.g.,
methodForGanesh, methodForGraphite). Do not overload a generic function
identifier with different backend-specific parameter types.

**Why.** Shared headers may be compiled in configurations where only one
rendering backend is enabled or linked. Overloading a generic method name with
different backend parameter types obscures separate link-time dependencies and
backend coupling. Qualifying the identifier makes backend dependencies clear at
call sites and clarifies that each implementation is backend-specific rather
than a generic polymorphic overload.

## Examples

Bad:

```cpp
// In shared header (e.g., src/text/gpu/GlyphVector.h):
std::tuple<bool, int> regenerateAtlas(int begin, int end, GrMeshDrawTarget*);
std::tuple<bool, int> regenerateAtlas(int begin, int end, skgpu::graphite::Recorder*);
```

Good:

```cpp
// In shared header (e.g., src/text/gpu/GlyphVector.h):
std::tuple<bool, int> regenerateAtlasForGanesh(int begin, int end, GrMeshDrawTarget*);
std::tuple<bool, int> regenerateAtlasForGraphite(int begin, int end, skgpu::graphite::Recorder*);
```

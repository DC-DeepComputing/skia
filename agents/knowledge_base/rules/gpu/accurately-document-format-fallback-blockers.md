---
name: skia/gpu/accurately-document-format-fallback-blockers
description: When documenting backend texture format fallbacks, external format mappings, or workarounds, state the underlying compatibility blocker instead of speculating about hypothetical mappings
rubric: readability
severity: nit
---

**Rule.** When adding or modifying explanatory comments for backend texture
format fallbacks, external texture wrappers, or driver workarounds, state the
concrete technical compatibility blocker (such as RGB versus RGBA color-type
mismatches, channel layout, or bit depth) that necessitates the workaround. Do
not write speculative comments suggesting hypothetical alternative format
assignments or unverified mappings.

**Why.** Speculating in comments that an incompatible format could probably be
mapped to an alternative standard format is misleading because different color
types and channel layouts have distinct alpha and transfer semantics. Explaining
the exact compatibility blocker clarifies why a workaround or external format is
required and guides future developers investigating native support.

## Examples

Bad:

```cpp
// According to Android documentation, the equivalent format is GL_RGB8, but that is 24bpp
// in Ganesh. While its VkFormat is RGBA8_UNORM, and could probably also treat it as GL_RGBA8,
// keep it as an external format.
return GrBackendFormats::MakeGLExternal();
```

Good:

```cpp
// According to Android documentation, the equivalent format is GL_RGB8, but that is 24bpp
// in Ganesh. For now keep it as an external format until we resolve RGB vs RGBA color-type
// compatibility.
return GrBackendFormats::MakeGLExternal();
```

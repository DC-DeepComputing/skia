---
name: skia/sksl/name-sksl-intrinsics-after-standard-glsl
description: When declaring built-in SkSL intrinsics, name them to match standard GLSL function names rather than backend-specific equivalents
rubric: readability
severity: should_fix
---

**Rule.** When introducing or modifying built-in SkSL intrinsics in modules or
compiler tables, name them to match standard GLSL built-in function names (such
as textureSize rather than backend-specific equivalents like WGSL's
textureDimensions). Preserve standard GLSL naming conventions across SkSL
intrinsic signatures, compiler tables, and backend code generators.

**Why.** SkSL is designed as a standardized variant of GLSL. Adhering to
standard GLSL intrinsic naming keeps SkSL and AGSL consistent with GLSL
specifications, simplifies porting existing GLSL shaders to SkSL, and avoids
leaking target-backend nomenclature into frontend shading language interfaces.

## Examples

Bad:

```cpp
// In src/sksl/sksl_gpu.sksl:
$pure uint2 textureDimensions(sampler2D s);
$pure uint2 textureDimensions($genTexture2D t);
```

Good:

```cpp
// In src/sksl/sksl_gpu.sksl:
$pure uint2 textureSize(sampler2D s);
$pure uint2 textureSize($genTexture2D t);
```

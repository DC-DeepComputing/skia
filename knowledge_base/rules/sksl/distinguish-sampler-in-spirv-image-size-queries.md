---
name: skia/sksl/distinguish-sampler-in-spirv-image-size-queries
description: When querying image dimensions in SkSL SPIR-V codegen, unwrap kSampler via OpImage and use OpImageQuerySizeLod instead of calling OpImageQuerySize directly
rubric: correctness
severity: blocking
---

**Rule.** When lowering image dimension queries (such as textureSize,
textureWidth, or textureHeight) in SkSL SPIR-V code generation, distinguish
between combined samplers (TypeKind::kSampler) and storage textures
(TypeKind::kTexture). For kSampler, extract the underlying OpTypeImage using
SpvOpImage (via writeExtractImage) and query dimensions using
SpvOpImageQuerySizeLod with LOD zero, rather than emitting SpvOpImageQuerySize
directly on the OpTypeSampledImage.

**Why.** SPIR-V requires OpImageQuerySize operands to be OpTypeImage, which
causes validation failures when passed an OpTypeSampledImage. Combined samplers
must be unwrapped via SpvOpImage and queried with SpvOpImageQuerySizeLod using
an explicit LOD level.

## Examples

Bad:

```cpp
SpvId dimsType = this->getType(callType);
SpvId image = this->writeExpression(*arguments[0], out);
this->writeInstruction(SpvOpImageQuerySize, dimsType, result, image, out);
```

Good:

```cpp
SpvId dimsType = this->getType(callType);
SpvId image = this->writeExpression(*arguments[0], out);
const Type& arg0Type = arguments[0]->type();
image = this->writeExtractImage(image, arg0Type, out);
if (arg0Type.typeKind() == Type::TypeKind::kSampler) {
    SpvId lodZero = this->writeOpConstant(*fContext.fTypes.fInt, 0);
    this->writeInstruction(SpvOpImageQuerySizeLod, dimsType, result, image, lodZero, out);
} else {
    this->writeInstruction(SpvOpImageQuerySize, dimsType, result, image, out);
}
```

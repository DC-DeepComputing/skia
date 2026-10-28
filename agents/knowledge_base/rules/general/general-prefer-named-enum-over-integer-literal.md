---
name: skia/general-prefer-named-enum-over-integer-literal
description: When handling enum values, mode flags, or type codes, use explicit named enum members or constants instead of magic numbers or comment labels
rubric: readability
severity: should_fix
---

**Rule.** Do not use raw integer literals or magic numbers (such as 0 or raw
case values) when checking, comparing, branching on, assigning, returning, or
passing values that represent enum constants, type codes, or mode flags. Define
and reference explicit named enum members (or named const/constexpr constants in
shaders where enums are unavailable) rather than relying on inline comment
annotations (such as /* kFoo */ case 2:). Additionally, when a canonical named
constant already exists for a composite flag combination or bitmask, reference
that existing constant rather than repeating or recomputing the bitwise OR
expression inline.

**Why.** Using raw numeric literals obscures semantic intent and leaves readers
uncertain which enum constant or mode is represented. Annotating magic numbers
with inline comments lacks compiler validation, risks silent desynchronization,
and makes refactoring error-prone. Similarly, repeating composite bitmask
expressions introduces code duplication and risks divergence if the canonical
constant definition changes.

## Examples

Bad:

```cpp
// Bad: returning raw integer literal 0 obscures that this represents a MaskFormat enum
unsigned short instanceFlags() const override {
    return 0;
}

// Bad: comparing against raw magic number instead of named enum constant
if (s.type == 50) {
    fImageAssetMap[s.slotID] = sk_make_sp<ImageAssetProxy>();
}

// Bad: using magic numbers with inline comment labels in switch cases
switch (colorSpace) {
    /* kLab   */ case 2:
    /* kOKLab */ case 3: color = unpremul(color); break;
    /* kLCH   */ case 4:
    /* kOKLCH */ case 5: color = $unpremul_polar(color); break;
}

// Bad: repeating composite bitmask expression inline when a dedicated constant exists
pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
```

Good:

```cpp
// Good: explicitly return the named enum member, cast to the return type if needed
unsigned short instanceFlags() const override {
    return (unsigned short)MaskFormat::kA8;
}

// Good: reference the explicit named enum member
if (s.type == SlotType::kImage) {
    fImageAssetMap[s.slotID] = sk_make_sp<ImageAssetProxy>();
}

// Good: define named constants and reference them in cases or conditions
const int kLab   = 2;
const int kOKLab = 3;
const int kLCH   = 4;
const int kOKLCH = 5;
switch (colorSpace) {
    case kLab:
    case kOKLab: color = unpremul(color); break;
    case kLCH:
    case kOKLCH: color = $unpremul_polar(color); break;
}

// Good: reuse the canonical named constant defined for the composite flags
pushConstantRange.stageFlags = VulkanResourceProvider::kIntrinsicConstantStageFlags;
```

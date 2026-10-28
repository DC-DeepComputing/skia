---
name: skia/gpu/resolve-vulkan-component-swizzle-identity
description: When converting or stringifying Vulkan component swizzles (VkComponentSwizzle), resolve VK_COMPONENT_SWIZZLE_IDENTITY to the channel's identity character ('r', 'g', 'b', 'a') rather than 'I'
rubric: correctness
severity: should_fix
---

**Rule.** When stringifying or converting Vulkan component swizzles
(VkComponentSwizzle), resolve VK_COMPONENT_SWIZZLE_IDENTITY to the corresponding
channel's identity character ('r', 'g', 'b', 'a') rather than returning a
generic 'I'. Conversion helpers should accept the channel's expected identity
character and return it for identity mappings.

**Why.** In the Vulkan specification, VK_COMPONENT_SWIZZLE_IDENTITY specifies
that a component maps to its own channel value (R to R, G to G, etc.).
Stringifying VK_COMPONENT_SWIZZLE_IDENTITY as a literal 'I' produces
non-canonical swizzle representations (e.g. 'IIII') that do not match explicit
identity mappings ('rgba'), causing inconsistencies in string comparisons,
pipeline keys, and debug labels.

## Examples

Bad:

```cpp
char VkSwizzleToStr(VkComponentSwizzle c) {
    switch (c) {
        case VK_COMPONENT_SWIZZLE_IDENTITY: return 'I';
        case VK_COMPONENT_SWIZZLE_ZERO:     return '0';
        case VK_COMPONENT_SWIZZLE_ONE:      return '1';
        case VK_COMPONENT_SWIZZLE_R:        return 'r';
        case VK_COMPONENT_SWIZZLE_G:        return 'g';
        case VK_COMPONENT_SWIZZLE_B:        return 'b';
        case VK_COMPONENT_SWIZZLE_A:        return 'a';
        default:                            return '?';
    }
}
```

Good:

```cpp
static constexpr char VkSwizzleToStr(VkComponentSwizzle c, char identityAnswer) {
    switch (c) {
        case VK_COMPONENT_SWIZZLE_IDENTITY: return identityAnswer;
        case VK_COMPONENT_SWIZZLE_ZERO:     return '0';
        case VK_COMPONENT_SWIZZLE_ONE:      return '1';
        case VK_COMPONENT_SWIZZLE_R:        return 'r';
        case VK_COMPONENT_SWIZZLE_G:        return 'g';
        case VK_COMPONENT_SWIZZLE_B:        return 'b';
        case VK_COMPONENT_SWIZZLE_A:        return 'a';
        default:                            return '?';
    }
}
// Called per channel:
// result += VkSwizzleToStr(components.r, 'r');
// result += VkSwizzleToStr(components.g, 'g');
```

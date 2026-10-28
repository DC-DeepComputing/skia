---
name: skia/dont-return-empty-braces-for-bool
description: When returning boolean values from functions, return explicit false or true literals rather than empty braces {} or integer literals 0 or 1
rubric: readability
severity: should_fix
---

**Rule.** Do not use empty braces {} or integer literals (such as 0 or 1) to
return a boolean value from a function; return an explicit false or true literal
instead. Reserve {} and value initialization for default-constructing structs,
containers, or std::optional values.

**Why.** Returning 0, 1, or empty braces {} from a function with a bool return
type relies on implicit conversion or scalar value-initialization, which
obscures intent and can mimic default-constructed structs or empty std::optional
returns. Explicitly returning false or true matches the declared bool signature,
avoids type confusion, and adheres to idiomatic C++ conventions.

## Examples

Bad:

```cpp
// Bad: returning empty braces or integer literals in bool functions
static bool can_direct_blit(const SkPaint& paint) {
    if (paint.getShader() || paint.getColorFilter() || paint.isDither()) {
        return {};
    }
    return true;
}

bool VulkanCaps::isSampleCountSupported(TextureFormat format, uint8_t count) const {
    if (!formatInfo.isDepthStencilSupported(...)) {
        return 0;
    }
    return 1;
}
```

Good:

```cpp
// Good: explicitly return false or true boolean literals
static bool can_direct_blit(const SkPaint& paint) {
    if (paint.getShader() || paint.getColorFilter() || paint.isDither()) {
        return false;
    }
    return true;
}

bool VulkanCaps::isSampleCountSupported(TextureFormat format, uint8_t count) const {
    if (!formatInfo.isDepthStencilSupported(...)) {
        return false;
    }
    return true;
}
```

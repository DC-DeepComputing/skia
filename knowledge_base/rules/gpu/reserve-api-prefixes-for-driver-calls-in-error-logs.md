---
name: skia/gpu/reserve-api-prefixes-for-driver-calls-in-error-logs
description: When logging GPU errors, use exact native API function names for driver calls and omit external API prefixes from internal Skia helper names
rubric: readability
severity: should_fix
---

**Rule.** In GPU backend error log messages (such as `SKIA_LOG_E` or
`SKGPU_LOG_E`), use exact, standard entry-point names for native driver calls
(e.g., `vkCreateImage failed`), and do not prefix internal Skia helper function
names with external graphics API names (e.g., use `AllocateAndBindImageMemory
failed` rather than `Vulkan AllocateAndBindImageMemory failed`). Reserve API
names and prefixes strictly for direct driver calls or standard specification
symbols.

**Why.** Using the exact native API name (e.g., `vkCreateImage`) immediately
clarifies that a failure originated in the driver and makes errors easily
searchable against driver specifications. Conversely, prefixing internal Skia
helpers with external API names falsely implies a direct driver call failed,
obscuring the true failure origin during log and crash triage.

## Examples

Bad:

```cpp
// Bad: Paraphrased driver call name
SKIA_LOG_E("Vulkan CreateImage failed (err: %d)", err);

// Bad: External API prefix on internal Skia helper
if (!skgpu::AllocateAndBindImageMemory(&alloc, image, props, hwbProps, hwb, vk, dev)) {
    SKIA_LOG_E("Vulkan AllocateAndBindImageMemory failed.");
    return {};
}
```

Good:

```cpp
// Good: Explicit standard driver API function name
SKIA_LOG_E("vkCreateImage failed (err: %d)", err);

// Good: Internal helper name without API prefix
if (!skgpu::AllocateAndBindImageMemory(&alloc, image, props, hwbProps, hwb, vk, dev)) {
    SKIA_LOG_E("AllocateAndBindImageMemory failed.");
    return {};
}
```

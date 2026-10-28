---
name: skia/api/omit-vulkan-hpp-headers
description: When updating or vendoring Vulkan headers in include/third_party/vulkan, omit C++ Vulkan-Hpp bindings (.hpp, .cppm) and only vendor required C headers (.h)
rubric: design
severity: should_fix
---

**Rule.** When updating or vendoring Vulkan headers under
include/third_party/vulkan, only vendor the C headers (.h) necessary for Skia's
Vulkan backend to compile. Do not check in C++ Vulkan-Hpp bindings (.hpp or
.cppm files such as vulkan.hpp or vulkan_structs.hpp) or reference them in build
files.

**Why.** Skia's Vulkan backend exclusively uses the C Vulkan API. Vendoring C++
Vulkan-Hpp headers introduces hundreds of thousands of lines of unused code and
high churn, causing unnecessary repository bloat and maintenance overhead.

## Examples

Bad:

```cpp
// Vendoring massive C++ Vulkan-Hpp bindings under include/third_party/vulkan/vulkan/:
include/third_party/vulkan/vulkan/vulkan.hpp
include/third_party/vulkan/vulkan/vulkan_structs.hpp
include/third_party/vulkan/vulkan/vulkan.cppm
```

Good:

```cpp
// Only vendoring necessary C Vulkan headers under include/third_party/vulkan/vulkan/:
include/third_party/vulkan/vulkan/vulkan.h
include/third_party/vulkan/vulkan/vulkan_core.h
include/third_party/vulkan/vulkan/vk_platform.h
```

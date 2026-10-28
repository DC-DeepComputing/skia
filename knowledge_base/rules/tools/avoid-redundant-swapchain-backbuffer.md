---
name: skia/tools/avoid-redundant-swapchain-backbuffer
description: When managing Vulkan swapchain presentation, store the acquired image index directly rather than maintaining a redundant backbuffer ring buffer wrapping the index
rubric: design
severity: should_fix
---

**Rule.** In Vulkan swapchain presentation contexts, store the acquired image
index returned by vkAcquireNextImageKHR directly into a scalar member rather
than maintaining a separate backbuffer tracking array or ring buffer that merely
wraps the image index. Use the stored image index directly to access per-image
swapchain resources across acquisition, draw submission, and presentation.

**Why.** When render completion semaphores and surfaces are tracked per
swapchain image, maintaining a separate backbuffer array or ring-buffer index
introduces redundant indirection and complicates synchronization state. Storing
the acquired image index directly streamlines state management and matches
Vulkan's image acquisition model.

## Examples

Bad:

```cpp
struct Backbuffer {
    uint32_t fImageIndex = -1;
};
Backbuffer* fBackbuffers;
uint32_t fCurrentBackbufferIndex;

Backbuffer* availableBackbuffer = this->getAvailableBackbuffer();
fAcquireNextImageKHR(fDevice, fSwapchain, UINT64_MAX, fAcquireSemaphore,
                     VK_NULL_HANDLE, &availableBackbuffer->fImageIndex);
```

Good:

```cpp
uint32_t fCurrentImageIndex;

fAcquireNextImageKHR(fDevice, fSwapchain, UINT64_MAX, fAcquireSemaphore,
                     VK_NULL_HANDLE, &fCurrentImageIndex);
SkSurface* surface = fImages[fCurrentImageIndex].fSurface.get();
```

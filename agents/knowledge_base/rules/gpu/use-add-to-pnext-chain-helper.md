---
name: skia/gpu/use-add-to-pnext-chain-helper
description: When chaining extension structures onto Vulkan structs, use the AddToPNextChain helper rather than manually wiring void* pNext pointers
rubric: readability
severity: should_fix
---

**Rule.** Use the AddToPNextChain helper rather than manually assigning and
threading void* pNext pointers when chaining extension structures onto Vulkan
structs.

**Why.** Manually updating and threading void* pNext pointers across conditional
extension structs is error-prone and can easily lead to missed links or cyclical
chains. AddToPNextChain prepends safely and includes compile-time and runtime
assertions that prevent pointer-to-pointer errors and uninitialized link state.

## Examples

Bad:

```cpp
void* pNext = nullptr;

VkProtectedSubmitInfo protectedSubmitInfo = {};
if (protectedContext == Protected::kYes) {
    protectedSubmitInfo.sType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
    protectedSubmitInfo.protectedSubmit = VK_TRUE;
    pNext = &protectedSubmitInfo;
}

VkFrameBoundaryEXT frameBoundary = {};
if (markFrameBoundary == MarkFrameBoundary::kYes) {
    frameBoundary.sType = VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT;
    frameBoundary.pNext = pNext;
    pNext = &frameBoundary;
}

VkSubmitInfo submitInfo = {};
submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
submitInfo.pNext = pNext;
```

Good:

```cpp
VkSubmitInfo submitInfo = {};
submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

VkProtectedSubmitInfo protectedSubmitInfo = {};
if (protectedContext == Protected::kYes) {
    protectedSubmitInfo.sType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
    protectedSubmitInfo.protectedSubmit = VK_TRUE;
    AddToPNextChain(&submitInfo, &protectedSubmitInfo);
}

VkFrameBoundaryEXT frameBoundary = {};
if (markFrameBoundary == MarkFrameBoundary::kYes) {
    frameBoundary.sType = VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT;
    AddToPNextChain(&submitInfo, &frameBoundary);
}
```

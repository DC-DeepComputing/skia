---
name: skia/avoid-redundant-empty-ternary-for-container-pointer
description: When passing container pointers to APIs or struct fields alongside an element count, pass .data() or .begin() directly without ternary empty-checks or &container.front()
rubric: readability
severity: should_fix
---

**Rule.** When passing contiguous container pointers to API functions or
populating struct pointer fields alongside an element count, pass container
.data() or .begin() directly. Do not wrap pointer retrieval in redundant ternary
empty checks (such as empty() ? nullptr : data()) or guard .front() accesses
with ternaries (such as empty() ? VK_NULL_HANDLE : &container.front()).

**Why.** In C++11 and later, container methods like .data() and .begin() safely
evaluate on empty containers without dereferencing or throwing, and APIs or
structs (such as Vulkan create info structs) that accept an explicit element
count do not dereference the pointer when the count is zero. Guarding pointer
access with ternary empty checks introduces redundant boilerplate, and taking
the address of .front() risks undefined behavior if the empty check is omitted
or refactored.

## Examples

Bad:

```cpp
// Direct function call argument:
CmdSetVertexInput(commandBuffer,
                  fVertexBindingDescriptions.size(),
                  fVertexBindingDescriptions.empty()
                          ? nullptr
                          : fVertexBindingDescriptions.begin());

// Vulkan create info struct field with front():
layoutCreateInfo.pBindings = bindingLayouts.empty() ? VK_NULL_HANDLE : &bindingLayouts.front();

// Populating struct / descriptor field:
vertexInputInfo->vertexBindingDescriptionCount = bindingDescs.size();
vertexInputInfo->pVertexBindingDescriptions =
        !bindingDescs.empty() ? bindingDescs.begin() : nullptr;

// Aggregate initialization:
SkFontArguments::Palette palette{
    fPaletteIndex,
    fPaletteOverrides.empty() ? nullptr : fPaletteOverrides.data(),
    static_cast<int>(fPaletteOverrides.size())
};
```

Good:

```cpp
// Direct function call argument:
CmdSetVertexInput(commandBuffer,
                  fVertexBindingDescriptions.size(),
                  fVertexBindingDescriptions.begin());

// Vulkan create info struct field with .data():
layoutCreateInfo.pBindings = bindingLayouts.data();

// Populating struct / descriptor field:
vertexInputInfo->vertexBindingDescriptionCount = bindingDescs.size();
vertexInputInfo->pVertexBindingDescriptions = bindingDescs.begin();

// Aggregate initialization:
SkFontArguments::Palette palette{
    fPaletteIndex,
    fPaletteOverrides.data(),
    static_cast<int>(fPaletteOverrides.size())
};
```

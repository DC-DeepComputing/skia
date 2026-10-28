---
name: skia/encapsulate-driver-handles-in-per-item-structs
description: When managing correlated per-item properties, driver handles, or multi-field records, encapsulate fields into a dedicated struct instead of parallel collections or flattened arrays
rubric: readability
severity: should_fix
---

**Rule.** Encapsulate correlated per-item properties, resources, or multi-field
records into a dedicated struct instead of maintaining parallel collections,
member arrays, or flattened primitive arrays that require manual stride
indexing. When exposing multi-attribute records across APIs or test helpers,
return a collection of named structs rather than packing multiple attributes
into a flat array of primitive integers. When interacting with bulk-query APIs
(such as vkGetSwapchainImagesKHR), query into a local temporary array and copy
the handles into the per-item structs during initialization.

**Why.** Maintaining parallel collections or packing multiple record attributes
into flat primitive arrays obscures field semantics, discards type safety, and
forces brittle manual stride or index synchronization. Defining a dedicated
struct establishes a single source of truth, preserves types and named member
access, and prevents parallel indexing and stride calculation bugs.

## Examples

Bad:

```cpp
// Parallel collections relying on implicit index synchronization
std::unique_ptr<skia_private::TArray<std::unique_ptr<SkCodec>>> fEmbeddedCodecs;
std::vector<sk_sp<SkData>> fBmpEntryData;

// Parallel member arrays maintained for bulk driver query APIs
struct SwapchainImageInfo {
    VkImageLayout fImageLayout;
    sk_sp<SkSurface> fSurface;
};
VkImage* fVkImages;
SwapchainImageInfo* fSwapchainImgInfo;

// Flattened primitive array requiring manual stride and index arithmetic
skia_private::TArray<uint32_t> getBufferInfo() {
    skia_private::TArray<uint32_t> info;
    for (const CopyRange& data : fData) {
        info.push_back(data.fTarget->fOffset);
        info.push_back(data.fUnalignedSize);
        info.push_back(data.fTarget->fSize);
        info.push_back(data.fRequiredAlignment);
    }
    return info;
}
```

Good:

```cpp
// Single collection of structs keeps correlated per-item fields in lockstep
struct EmbeddedImage {
    std::unique_ptr<SkCodec> fCodec;
    sk_sp<const SkData> fBmpEntryData;
};
std::vector<EmbeddedImage> fEmbeddedImages;

// Per-item struct stores driver handle directly; temporary used for bulk queries
struct SwapchainImage {
    VkImage fVkImage = VK_NULL_HANDLE;
    VkImageLayout fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    sk_sp<SkSurface> fSurface = nullptr;
};
skia_private::AutoTArray<SwapchainImage> fImages;

// Named struct preserves field types and provides direct member access
struct StaticVertexCopyRanges {
    uint32_t fOffset;
    size_t fUnalignedSize;
    size_t fSize;
    size_t fRequiredAlignment;
};
skia_private::TArray<StaticVertexCopyRanges> testingOnly_getCopyRanges() const {
    skia_private::TArray<StaticVertexCopyRanges> info;
    for (const CopyRange& data : fData) {
        info.push_back({data.fTarget->fOffset,
                        data.fUnalignedSize,
                        data.fTarget->fSize,
                        data.fRequiredAlignment});
    }
    return info;
}
```

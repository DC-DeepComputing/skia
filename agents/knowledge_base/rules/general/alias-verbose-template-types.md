---
name: skia/alias-verbose-template-types
description: When repeatedly referencing verbose templates or deeply qualified types and enums, introduce a scoped type alias and use existing aliases consistently
rubric: readability
severity: nit
---

**Rule.** Introduce a descriptive type alias (via `using Alias = ...;`) for
verbose template signatures (such as multi-element `std::tuple` types or
`std::unique_ptr` with custom deleters) or deeply qualified types and enums
(such as FFI bridges or nested namespaces) that appear repeatedly. Scope the
alias appropriately: declare it within the class or interface when shared across
member declarations, or locally inside a function or block when only needed
locally. Furthermore, where a local or scoped type alias already exists, use it
consistently throughout the function or scope instead of writing out redundant
fully-qualified type or enum names.

**Why.** Repeating complex template signatures or lengthy qualified type names
forces statements to break awkwardly across lines under column limits and
clutters function bodies and interfaces. Scoping a type alias to the class,
interface, or local function body makes code concise and readable without
causing global namespace pollution, and adhering consistently to in-scope
aliases eliminates visual noise and style inconsistency.

## Examples

Bad:

```cpp
// Verbose multi-element tuple repeated across method declarations
std::tuple<BufferWriter, BindBufferInfo, BufferSubAllocator> getMappedVertexBuffer(
        size_t count, size_t stride, size_t reservedCount=0, size_t alignment=1);
std::tuple<BufferWriter, BindBufferInfo, BufferSubAllocator> getMappedIndexBuffer(
        size_t count);

// Verbose custom-deleter template type in local declaration
std::unique_ptr<crabbyavif::avifImage, decltype(&crabbyavif::crabby_avifImageDestroy)>
        cropped_image{nullptr, crabbyavif::crabby_avifImageDestroy};

// Redundant fully-qualified enum names despite existing in-scope alias
using ColorSpace = SkGradientShader::Interpolation::ColorSpace;
switch (cs) {
    case ColorSpace::kOKLab:
        return SkColorSpace::MakeSRGBLinear();
    case SkGradientShader::Interpolation::ColorSpace::kDisplayP3:
        return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3);
}
```

Good:

```cpp
// Define an alias at class/interface scope for repeated composite return types
using MappedAllocationInfo = std::tuple<BufferWriter, BindBufferInfo, BufferSubAllocator>;
MappedAllocationInfo getMappedVertexBuffer(
        size_t count, size_t stride, size_t reservedCount=0, size_t alignment=1);
MappedAllocationInfo getMappedIndexBuffer(size_t count);

// Define a function-scoped alias to keep local declarations concise
using AvifImagePtr =
        std::unique_ptr<crabbyavif::avifImage, decltype(&crabbyavif::crabby_avifImageDestroy)>;
AvifImagePtr cropped_image{nullptr, crabbyavif::crabby_avifImageDestroy};

// Consistently use the existing local alias within the function scope
using ColorSpace = SkGradientShader::Interpolation::ColorSpace;
switch (cs) {
    case ColorSpace::kOKLab:
        return SkColorSpace::MakeSRGBLinear();
    case ColorSpace::kDisplayP3:
        return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3);
}
```

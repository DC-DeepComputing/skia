---
name: skia/omit-redundant-empty-brace-member-initialization
description: When declaring member or local variables of default-constructible types (such as containers, strings, sk_sp, or std::optional), omit redundant '{}', '= {}', '= nullptr', '= std::nullopt', or '("")'
rubric: readability
severity: nit
---

**Rule.** Do not add redundant '{}', '= {}', '= nullptr', '= std::nullopt', or
empty string literals ('("")' or '= ""') to member or variable declarations
whose types already provide default constructors that initialize an empty or
null state (such as skia_private::AutoTArray, TArray, sk_sp, std::unique_ptr,
SkString, std::string, std::vector, or std::optional). Rely on standard C++
default initialization to construct the object in its empty, null, or disengaged
initial state.

**Why.** In C++, member and local variables of default-constructible classes are
automatically default-constructed to an empty state (e.g., an empty container or
string, null sk_sp/unique_ptr, or disengaged std::optional). Adding '{}', '=
{}', '= nullptr', '= std::nullopt', or '("")' introduces redundant syntactic
noise, can needlessly invoke conversion constructors, and misleads readers into
expecting custom initialization.

## Examples

Bad:

```cpp
// Bad: Redundant '{}', '= {}', '= nullptr', '= std::nullopt', or '("")' on default-constructible members/variables
class Encoder {
    skia_private::AutoTArray<SwapchainImage> fImages = {};
    sk_sp<SkSurface> fSurface = {};
    std::vector<ChildData> fChildren{};
    std::vector<int> fIndices = {};
    std::optional<SkImageInfo> fSrcInfo = std::nullopt;
    SkString fName = "";
};

void process() {
    sk_sp<SkColorSpace> inputSpace = nullptr;
    sk_sp<SkColorSpace> outputSpace = nullptr;
    SkString label("");
    std::string title("");
}
```

Good:

```cpp
// Good: Rely on default initialization to construct empty, null, or disengaged state
class Encoder {
    skia_private::AutoTArray<SwapchainImage> fImages;
    sk_sp<SkSurface> fSurface;
    std::vector<ChildData> fChildren;
    std::vector<int> fIndices;
    std::optional<SkImageInfo> fSrcInfo;
    SkString fName;
};

void process() {
    sk_sp<SkColorSpace> inputSpace;
    sk_sp<SkColorSpace> outputSpace;
    SkString label;
    std::string title;
}
```

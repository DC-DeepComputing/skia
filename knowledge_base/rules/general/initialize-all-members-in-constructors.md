---
name: skia/initialize-all-members-in-constructors
description: When defining constructors or class members, initialize fields—including raw pointers, primitives, containers, and smart pointers—via in-class defaults or initializer lists, avoiding uninitialized reads and body assignments
rubric: correctness
severity: blocking
---

**Rule.** When defining constructors or adding class members, explicitly
initialize every member variable via in-class default initializers (such as =
nullptr, = 0, or = false) or constructor initializer lists, but not both. Never
leave raw pointer, primitive, or enum members uninitialized for subsequent
methods to populate. Initialize dynamic heap allocations (sk_make_sp<T>() or
std::make_unique<T>()) in constructor initializer lists in .cpp files rather
than header declarations. Avoid omitting fields, redundantly initializing in
both places, or populating fields inside constructor bodies: initialize
containers via braced lists or range constructors, and move smart pointers
(std::unique_ptr, sk_sp) directly in initializer lists without defensive null
checks.

**Why.** In C++, primitive, enum, and raw pointer member variables lacking
default initializers remain uninitialized with indeterminate values if omitted
from in-class defaults or constructor initializer lists, causing undefined
behavior or MSan violations when read before or between subsequent method calls.
Initializing members in both places risks maintenance divergence, while
populating fields or containers inside constructor bodies performs redundant
default construction and unnecessary copies. Moving smart pointers directly in
initializer lists safely handles null without defensive branching or deferred
body assignments.

## Examples

Bad:

```cpp
// Bad: Leaving pointer or primitive member uninitialized for later setters/methods.
class AnimationBuilder {
    mutable const char* fPropertyObserverContext;
    bool fHasFinished;
};

// Bad: Omitting member leaves enum uninitialized; assigning in body is redundant.
PaintParams::PaintParams(const SkPaint& paint, Coverage coverage, TextureFormat targetFormat)
        : fColor(paint.getColor4f())
        , fRendererCoverage(coverage) {}

// Bad: Branching on null to assign or reset smart pointer in constructor body.
ParagraphBuilderImpl::ParagraphBuilderImpl(std::unique_ptr<SkUnicode> unicode) {
    if (unicode != nullptr) {
        fUnicode = std::move(unicode);
    } else {
        fUnicode.reset();
    }
}

// Bad: Initializing members both in-class and in constructor initializer list.
int32_t fActiveSlot = 0;
AlphaAtlasManager::AlphaAtlasManager(Recorder* recorder)
        : fRecorder(recorder), fActiveSlot(0) {}
```

Good:

```cpp
// Good: Raw pointers and primitives default-initialized at declaration.
class AnimationBuilder {
    mutable const char* fPropertyObserverContext = nullptr;
    bool fHasFinished = false;
};

// Good: All members initialized directly in constructor initializer list.
PaintParams::PaintParams(const SkPaint& paint, Coverage coverage, TextureFormat targetFormat)
        : fColor(paint.getColor4f())
        , fRendererCoverage(coverage)
        , fTargetFormat(targetFormat) {}

// Good: Smart pointer moved directly in initializer list (safely handles null).
ParagraphBuilderImpl::ParagraphBuilderImpl(std::unique_ptr<SkUnicode> unicode)
        : fUnicode(std::move(unicode)) {}

// Good: Initialize members in only one place (either in-class or initializer list).
int32_t fActiveSlot;
AlphaAtlasManager::AlphaAtlasManager(Recorder* recorder)
        : fRecorder(recorder), fActiveSlot(0) {}
```

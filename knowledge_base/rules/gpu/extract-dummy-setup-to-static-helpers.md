---
name: skia/gpu/extract-dummy-setup-to-static-helpers
description: When synthesizing temporary dummy objects to query derived metadata, extract multi-step setup into file-local static helpers returning owning smart pointers
rubric: design
severity: should_fix
---

**Rule.** When complex member functions (such as pipeline key builders) require
synthesizing temporary dummy objects solely to probe derived metadata or color
transforms, extract the multi-step setup into a file-local static helper in an
anonymous namespace. Have the helper return an owning smart pointer (such as
sk_sp<T>) rather than a raw pointer to ensure the derived object outlives the
destruction of the synthesized temporary.

**Why.** Inlining multi-line boilerplate to synthesize dummy objects clutters
the caller's primary execution flow and obscures key-generation logic. Isolating
dummy setup in a named file-local helper clarifies caller intent, and returning
an owning smart pointer prevents dangling pointer bugs when helper-local
temporaries are destroyed.

## Examples

Bad:

```cpp
void PrecompileGradientShader::addToKey(const KeyContext& keyContext, ...) const {
    ...
    constexpr SkPoint pts[2] = {{0.f, 0.f}, {1.f, 0.f}};
    constexpr SkColor4f colors[2] = {SkColors::kBlack, SkColors::kWhite};
    constexpr float pos[2] = {0.f, 1.f};
    SkLinearGradient shader(pts, {colors, nullptr, pos, 2, SkTileMode::kClamp, fInterpolation});
    SkColor4fXformer xformedColors(&shader, keyContext.dstColorInfo().colorSpace());
    const SkColorSpace* intermediateCS = xformedColors.fIntermediateColorSpace.get();
    ...
}
```

Good:

```cpp
namespace {

sk_sp<SkColorSpace> get_gradient_intermediate_cs(SkColorSpace* dstColorSpace,
                                                 SkGradientShader::Interpolation interpolation) {
    constexpr SkPoint pts[2] = {{0.f, 0.f}, {1.f, 0.f}};
    constexpr SkColor4f colors[2] = {SkColors::kBlack, SkColors::kWhite};
    constexpr float pos[2] = {0.f, 1.f};
    SkLinearGradient shader(pts, {colors, nullptr, pos, 2, SkTileMode::kClamp, interpolation});

    SkColor4fXformer xformedColors(&shader, dstColorSpace);
    return xformedColors.fIntermediateColorSpace;
}

}  // anonymous namespace

void PrecompileGradientShader::addToKey(const KeyContext& keyContext, ...) const {
    ...
    sk_sp<SkColorSpace> intermediateCS = get_gradient_intermediate_cs(
            keyContext.dstColorInfo().colorSpace(), fInterpolation);
    ...
}
```

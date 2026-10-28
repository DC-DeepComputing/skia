---
name: skia/modules/qualify-text-shaping-identifiers
description: In vector graphics modules like SVG or Skottie, qualify font and glyph shaping APIs and variables as TextShaping rather than unqualified Shaping
rubric: readability
severity: should_fix
---

**Rule.** In vector graphics and animation modules where 'shape' commonly refers
to 2D geometric shapes, paths, or layers (such as modules/svg and
modules/skottie), explicitly qualify font and glyph shaping APIs, namespaces,
builder methods, and member variables as 'TextShaping' (e.g.,
setTextShapingFactory, fTextShapingFactory) rather than unqualified 'Shaping'.
Reserve unqualified 'Shape' or 'Shaping' for vector path and geometric primitive
operations.

**Why.** In vector graphics frameworks like SVG and Lottie/Skottie, 'shape'
primarily refers to geometric elements such as paths, rects, contours, and shape
layers. Using unqualified 'Shaping' for font glyph shaping creates ambiguity
with vector path operations, whereas 'TextShaping' clarifies the subsystem
domain and maintains consistency across builder APIs.

## Examples

Bad:

```cpp
namespace SkSVGShaping {
using MakeShaperCallback = std::function<std::unique_ptr<SkShaper>(sk_sp<SkFontMgr>)>;
}

class SK_API SkSVGDOM::Builder {
    Builder& setShapingFactory(sk_sp<SkShapers::Factory>);
    sk_sp<SkShapers::Factory> fShapingFactory;
};
```

Good:

```cpp
namespace SkSVGTextShaping {
using MakeShaperCallback = std::function<std::unique_ptr<SkShaper>(sk_sp<SkFontMgr>)>;
}

class SK_API SkSVGDOM::Builder {
    Builder& setTextShapingFactory(sk_sp<SkShapers::Factory>);
    sk_sp<SkShapers::Factory> fTextShapingFactory;
};
```

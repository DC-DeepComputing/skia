---
name: skia/tests/use-default-typeface-for-shaping-tests
description: When writing tests exercising font shaping, text layout tolerances, or font manager integration, use ToolUtils::DefaultTypeface() rather than synthetic ToolUtils::DefaultPortableTypeface()
rubric: testing
severity: should_fix
---

**Rule.** In unit tests and benchmarks exercising font shaping, text layout
tolerances, or font manager features, obtain typefaces using
ToolUtils::DefaultTypeface() (or ToolUtils::DefaultFont()) rather than
ToolUtils::DefaultPortableTypeface() (or ToolUtils::DefaultPortableFont()).
Reserve portable fonts for Golden Master (GM) tests where platform-independent
pixel determinism is specifically required.

**Why.** ToolUtils::DefaultPortableTypeface() provides a synthetic portable font
designed for deterministic cross-platform GM rendering, but lacks realistic font
metrics and OpenType tables. Using it in font shaping or layout tests bypasses
the compiled-in font manager (TestFontMgr()) and shaper backend, leaving real
shaping paths unexercised or causing false tolerance failures.

## Examples

Bad:

```cpp
// In font shaping or layout tests:
sk_sp<SkTypeface> typeface = ToolUtils::DefaultPortableTypeface();
auto shaper = SkShapers::Primitive::Make();
skottie::Shaper::Shape(text, typeface, ...);
```

Good:

```cpp
// In font shaping or layout tests:
sk_sp<SkTypeface> typeface = ToolUtils::DefaultTypeface();
auto shaper = SkShapers::Primitive::Make();
skottie::Shaper::Shape(text, typeface, ...);
```

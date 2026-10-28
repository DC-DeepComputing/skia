---
name: skia/ports/subclass-proxy-for-platform-typefaces
description: When instantiating or wrapping platform typefaces in SkFontMgr ports, delegate creation to MakeFromStream and subclass SkTypeface_proxy rather than instantiating or subclassing SkTypeface_FreeTypeStream
rubric: readability
severity: should_fix
---

**Rule.** When implementing platform-specific typefaces or font managers,
delegate typeface creation from streams to standard factory methods (such as
scanner->MakeFromStream() or SkTypeface_FreeType::MakeFromStream()) and subclass
SkTypeface_proxy if platform-specific wrapping is required. Do not directly
instantiate or subclass backend-specific classes like SkTypeface_FreeTypeStream,
manually construct SkFontData, or invoke FreeType-specific scanner helpers.

**Why.** Direct instantiation or subclassing of SkTypeface_FreeTypeStream
bypasses font scanner metadata parsing (such as style, family name, and
variation axis resolution) and tightly couples font managers to FreeType,
preventing support for alternative font scanners like Fontations. Delegating
stream instantiation to scanner->MakeFromStream() and wrapping via
SkTypeface_proxy preserves automated metadata extraction and maintains clean
backend abstraction.

## Examples

Bad:

```cpp
// Direct instantiation with manual SkFontData construction:
auto data = std::make_unique<SkFontData>(std::move(stream), index, 0, nullptr, 0, nullptr, 0);
fProxy = sk_make_sp<SkTypeface_FreeTypeStream>(std::move(data), familyName, style, isFixedPitch);

// Subclassing backend stream and manually computing axes:
class SkTypeface_Fuchsia : public SkTypeface_FreeTypeStream { ... };
sk_sp<SkTypeface> CreateTypeface(SkFontScanner* scanner, std::unique_ptr<SkStreamAsset> stream,
                                 const SkFontArguments& args, TypefaceId id) {
    SkFontScanner_FreeType::computeAxisValues(axisDefs, curPos, pos, axisVals, name, &style);
    auto data = std::make_unique<SkFontData>(std::move(stream), ...);
    return sk_make_sp<SkTypeface_Fuchsia>(std::move(data), style, isFixedPitch, name, id);
}
```

Good:

```cpp
// Delegating to MakeFromStream factory helper:
fProxy = scanner->MakeFromStream(SkStream::MakeFromFile(filename), SkFontArguments());

// Subclassing SkTypeface_proxy to wrap standard typeface instantiation:
class SkTypeface_Fuchsia : public SkTypeface_proxy {
public:
    static sk_sp<SkTypeface_Fuchsia> Make(sk_sp<SkTypeface> realFace, TypefaceId id) {
        return sk_sp(new SkTypeface_Fuchsia(std::move(realFace), id));
    }
private:
    SkTypeface_Fuchsia(sk_sp<SkTypeface> realFace, TypefaceId id)
            : SkTypeface_proxy(std::move(realFace), realFace->fontStyle(), realFace->isFixedPitch())
            , fId(id) {}
    TypefaceId fId;
};
```

---
name: skia/api/split-speculative-client-apis
description: When modifying public headers or implementing new features, omit speculative methods, redundant overloads, unused interface overrides, and unrelated embedder constants, keeping public APIs minimal
rubric: change_scope
severity: should_fix
---

**Rule.** When implementing new features, modifying public headers, or
upstreaming code from embedders, keep API additions strictly minimal and focused
on active client requirements. Do not add uncalled client methods, redundant
getter overloads (such as out-parameter getters when return-by-value exists),
speculative optional interface overrides (such as subset decoding when only
streaming decode is needed), or unrelated embedder constants and aliases. Split
independent features and future client integrations into separate follow-up CLs.

**Why.** Public headers define Skia's external client contract, and public
methods or constants cannot easily be removed once exposed to external
embedders. Adding speculative methods, redundant overloads, or bundled
independent features inflates review scope, increases API maintenance
obligations, and risks introducing dead code before downstream requirements are
known.

## Examples

Bad:

```cpp
// 1. Redundant getter overloads / speculative public methods:
class SkTypeface : public SkRefCnt {
public:
    SkFontStyle fontStyle() const;
    void getFontStyle(SkFontStyle* fontStyle) const;
};

// 2. Bundling independent features (incremental + speculative subset decoding):
class SkBmpRustCodec final : public SkCodec {
    bool onGetValidSubset(SkIRect* desiredSubset) const override;
    bool onDimensionsSupported(const SkISize& requested) override;
    Result onStartIncrementalDecode(...) override;
};

// 3. Speculative uncalled downstream-client methods on a new codec:
class SkIcoRustCodec : public SkCodec {
public:
    struct HotSpot { int16_t x, y; };
    bool isCursor() const { return fIsCursor; }
    HotSpot getHotSpot(int index) const;
};

// 4. Upstreaming unrelated embedder constants into public headers:
namespace SkNamedPrimaries {
static constexpr SkColorSpacePrimaries kSRGB = kRec709;
static constexpr SkColorSpacePrimaries kWideGamutColorSpin = { ... };
}
```

Good:

```cpp
// 1. Keep public API minimal; avoid redundant or speculative getter overloads:
class SkTypeface : public SkRefCnt {
public:
    SkFontStyle fontStyle() const;
};

// 2. CL focused strictly on required feature (incremental decoding), omitting unused subset overrides:
class SkBmpRustCodec final : public SkCodec {
    Result onStartIncrementalDecode(...) override;
    Result onIncrementalDecode(...) override;
};

// 3. Keep initial class focused strictly on core decoding; defer client-specific extensions:
class SkIcoRustCodec : public SkCodec {
public:
    static bool IsIco(const void*, size_t);
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);
};

// 4. Only upstream constants and enums required for the feature (e.g. CICP):
namespace SkNamedPrimaries {
enum class CicpId : uint8_t {
    kRec709 = 1,
    kRec470SystemM = 4,
};
}
```

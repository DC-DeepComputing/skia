---
name: skia/modules/modules-override-specific-resource-provider-loader
description: When implementing skresources::ResourceProvider subclasses for specific asset types, override dedicated loaders like loadImageAsset directly rather than the generic load fallback
rubric: design
severity: should_fix
---

**Rule.** When implementing a skresources::ResourceProvider subclass that
handles a specific asset type (such as image assets, audio, or typefaces),
directly override the dedicated loader method (e.g., loadImageAsset(),
loadAudioAsset(), or loadTypeface()). Do not override the generic load()
fallback or route specialized loaders through load() with intermediate decoding
when only a single asset category is managed.

**Why.** ResourceProvider::load() returns raw SkData as a general fallback
primarily intended for nested animations and generic payloads. Overriding load()
to serve specific asset types introduces unnecessary abstraction layers,
intermediate data wrapping, and redundant decoding passes compared to directly
providing and caching typed asset instances.

## Examples

Bad:

```cpp
class CustomImageProvider final : public skresources::ResourceProvider {
public:
    sk_sp<SkData> load(const char[], const char slot_name[]) const override {
        return fAssetDataMap.at(slot_name);
    }

    sk_sp<skresources::ImageAsset> loadImageAsset(const char[],
                                                  const char slot_name[],
                                                  const char[]) const override {
        if (auto data = this->load(nullptr, slot_name)) {
            return skresources::MultiFrameImageAsset::Make(data, true);
        }
        return nullptr;
    }
};
```

Good:

```cpp
class CustomImageProvider final : public skresources::ResourceProvider {
public:
    sk_sp<skresources::ImageAsset> loadImageAsset(const char[],
                                                  const char slot_name[],
                                                  const char[]) const override {
        return fImageAssetMap.at(slot_name);
    }
};
```

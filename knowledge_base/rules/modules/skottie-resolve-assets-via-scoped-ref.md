---
name: skia/modules/skottie-resolve-assets-via-scoped-ref
description: When resolving footage or image assets via ScopedAssetRef and loadFootageAsset, verify loadFootageAsset returns non-null before accessing fAsset and avoid inline parsing
rubric: correctness
severity: blocking
---

**Rule.** In Skottie layers and effects, resolve top-level 'refId' footage and
image assets using AnimationBuilder::ScopedAssetRef and
AnimationBuilder::loadFootageAsset instead of embedding inline asset
dictionaries or calling fResourceProvider->loadImageAsset directly. Always check
that loadFootageAsset returns a non-null pointer before dereferencing fAsset or
fSize, handling null results gracefully.

**Why.** Embedding inline asset properties violates Lottie schema conventions,
whereas ScopedAssetRef and loadFootageAsset handle asset deduplication, cycle
detection, and slot overrides. Because loadFootageAsset returns nullptr when
asset properties are missing or loading fails, dereferencing its result without
a null check causes null pointer dereferences.

## Examples

Bad:

```cpp
// Unchecked dereference after loadFootageAsset:
const auto* asset_info = abuilder.loadFootageAsset(*footageAsset);
auto asset = asset_info->fAsset;

// Or inline parsing bypassing Skottie asset cache and cycle detection:
auto asset = abuilder.fResourceProvider->loadImageAsset(path->begin(),
                                                        name->begin(),
                                                        id->begin());
```

Good:

```cpp
// Resolve via ScopedAssetRef and verify non-null before dereferencing:
const skjson::ObjectValue* jimageRef = (*jprop)["v"];
const AnimationBuilder::ScopedAssetRef footageAsset(&abuilder, *jimageRef);
const auto* asset_info = abuilder.loadFootageAsset(*footageAsset);
if (asset_info && asset_info->fAsset) {
    auto frameData = asset_info->fAsset->getFrameData(0);
}
```

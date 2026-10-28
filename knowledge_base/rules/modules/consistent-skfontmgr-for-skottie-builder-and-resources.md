---
name: skia/modules/consistent-skfontmgr-for-skottie-builder-and-resources
description: When configuring Skottie animations with both a ResourceProvider and Animation::Builder, pass the same SkFontMgr instance to both to ensure consistent font resolution
rubric: correctness
severity: should_fix
---

**Rule.** When configuring a Skottie animation using both
skresources::ResourceProvider (such as DataURIResourceProviderProxy) and
skottie::Animation::Builder, pass the same SkFontMgr instance to both components
via the resource provider factory and Builder::setFontManager(). Avoid passing
disparate font managers or falling back to default/test font managers on only
one of the components.

**Why.** Skottie uses the resource provider's font manager to decode external or
data-URI font assets and the builder's font manager to resolve system typefaces
and fallback fonts. Passing mismatched SkFontMgr instances can cause
inconsistent font fallback resolution, glyph mismatches, and redundant font
manager instantiation.

## Examples

Bad:

```cpp
auto rp = skresources::CachingResourceProvider::Make(
        skresources::DataURIResourceProviderProxy::Make(
                skresources::FileResourceProvider::Make(SkOSPath::Dirname(FLAGS_input[0]),
                                                        predecode),
                predecode,
                fontMgr));
auto anim = skottie::Animation::Builder()
        .setFontManager(ToolUtils::TestFontMgr())
        .setResourceProvider(rp)
        .make(static_cast<const char*>(data->data()), data->size());
```

Good:

```cpp
auto rp = skresources::CachingResourceProvider::Make(
        skresources::DataURIResourceProviderProxy::Make(
                skresources::FileResourceProvider::Make(SkOSPath::Dirname(FLAGS_input[0]),
                                                        predecode),
                predecode,
                fontMgr));
auto anim = skottie::Animation::Builder()
        .setFontManager(fontMgr)
        .setResourceProvider(rp)
        .make(static_cast<const char*>(data->data()), data->size());
```

---
name: skia/register-global-factories-at-startup
description: When registering global deserializer or codec factories, register them upfront during process startup rather than lazily in getters, drawing routines, or deserializers
rubric: safety
severity: should_fix
---

**Rule.** Do not register global deserializer or codec factories (such as
SkTypeface::Register or SkCodecs::Register) lazily inside on-demand getters,
utility helpers, drawing methods, or deserialization callbacks. Register global
factories upfront in explicit process or harness initialization routines
alongside SkGraphics::Init(), or pass custom SkDeserialProcs (e.g.,
fImageDataProc or fImageProc) directly to deserialization APIs.

**Why.** SkCodecs::Register and SkTypeface::Register mutate unsynchronized
global registries and are not thread-safe, causing data races when called
concurrently during rendering or deserialization. Lazily registering factories
also introduces initialization-order races across test harnesses and creates
unexpected global side effects.

## Examples

Bad:

```cpp
// Bad: Lazily registering global factory inside utility helper
sk_sp<SkFontMgr> TestFontMgr() {
    SkTypeface::Register(SkTypeface_Fontations::FactoryId,
                         SkTypeface_Fontations::MakeFromStream);
    return ToolUtils::MakeFontMgr();
}

// Bad: Calling SkCodecs::Register dynamically during drawing or deserialization
if (slugBytes != nullptr) {
    SkCodecs::Register(SkPngDecoder::Decoder());
    auto slug = sktext::gpu::Slug::Deserialize(
            slugBytes->data(), slugBytes->size(), &fStrikeClient, {});
    this->drawSlug(slug.get());
}
```

Good:

```cpp
// Good: Register upfront during harness/process startup (e.g. dm, viewer main)
SkGraphics::Init();
ToolUtils::RegisterAvailableTypefaceFactories();
SkCodecs::Register(SkPngDecoder::Decoder());

// Good: Supply custom SkDeserialProcs locally without mutating global state
if (slugBytes != nullptr) {
    SkDeserialProcs procs;
    procs.fImageProc = [](const void* bytes, size_t length, void*) -> sk_sp<SkImage> {
        auto data = SkData::MakeWithoutCopy(bytes, length);
        auto codec = SkPngDecoder::Decode(data, nullptr);
        return codec ? SkCodecs::DeferredImage(std::move(codec)) : nullptr;
    };
    auto slug = sktext::gpu::Slug::Deserialize(
            slugBytes->data(), slugBytes->size(), &fStrikeClient, procs);
    this->drawSlug(slug.get());
}
```

---
name: skia/api/encapsulate-internal-options-in-options-priv
description: When adding internal tuning or test-only settings to public Graphite options structs, encapsulate them in companion *OptionsPriv structs in src/ referenced via pointer
rubric: design
severity: should_fix
---

**Rule.** Do not expose internal tuning parameters, buffer sizes, or test-only
settings directly on public Graphite options structs (such as RecorderOptions or
ContextOptions). Instead, forward-declare a companion internal struct
(*OptionsPriv) in the public header, reference it via a non-owning raw pointer
initialized to nullptr, and define the options struct in
src/gpu/graphite/*OptionsPriv.h for use by internal tools and test frameworks.

**Why.** Public options structs in include/gpu/graphite/ define client-facing
configuration contracts. Exposing internal tuning parameters, buffer limits, or
test-only flags bloats the client API, exposes unstable implementation details,
and creates false expectations of client configurability. Isolating private
options in internal companion structs confines test and benchmarking overrides
to Skia internals.

## Examples

Bad:

```cpp
// include/gpu/graphite/Recorder.h
struct SK_API RecorderOptions final {
    sk_sp<ImageProvider> fImageProvider;

    // Bad: Exposing internal buffer sizing and tuning directly in public API
    struct BufferSizes {
        uint32_t fVertexBufferMinSize;
    };
    std::optional<BufferSizes> fBufferSizes = std::nullopt;
};
```

Good:

```cpp
// include/gpu/graphite/Recorder.h
struct RecorderOptionsPriv;

struct SK_API RecorderOptions final {
    sk_sp<ImageProvider> fImageProvider;

    // Good: Private options isolated in internal struct, referenced via pointer
    RecorderOptionsPriv* fRecorderOptionsPriv = nullptr;
};

// src/gpu/graphite/RecorderOptionsPriv.h
struct RecorderOptionsPriv {
    std::optional<DrawBufferManager::BufferSizes> fBufferSizes = std::nullopt;
};
```

---
name: skia/api/avoid-single-value-public-api-parameters
description: When declaring public APIs or options structs, omit unimplemented, obsolete, or single-valued parameters, and encapsulate internal implementation constants within consuming classes
rubric: design
severity: should_fix
---

**Rule.** Do not add parameters to public API functions if the underlying
implementation only supports a single fixed value, and do not expose
configuration fields in public options structs (such as Options,
RecorderOptions, or ContextOptions) if the underlying feature is unimplemented
or obsolete. Furthermore, do not place internal implementation constants or
capacity limits inside public options structs; encapsulate them as private
static constexpr members within the class that uses them.

**Why.** Public options structs and signatures define external client
configuration surfaces; exposing single-valued parameters, unimplemented fields,
or obsolete options creates misleading silent no-ops or false expectations of
configurability. Placing internal constants or capacity limits inside options
structs pollutes public APIs and leaks internal sizing choices that should
remain private implementation details of consuming classes.

## Examples

Bad:

```cpp
// include/core/SkColorSpace.h: parameter only supports matrix_coefficients == 0
static sk_sp<SkColorSpace> MakeCICP(uint8_t color_primaries,
                                    uint8_t transfer_characteristics,
                                    uint8_t matrix_coefficients,
                                    uint8_t full_range_flag);

// include/encode/SkPngRustEncoder.h: fHdrMetadata exposed without implementation
struct Options {
    sk_sp<SkDataTable> fComments;
    skhdr::Metadata fHdrMetadata; // Unimplemented; silently ignored!
};

// include/gpu/graphite/ContextOptions.h: legacy VMA removed, but tuning field retained
struct ContextOptions {
    std::optional<uint64_t> fVulkanVMALargeHeapBlockSize;
};

// include/gpu/graphite/Recorder.h: internal constant in public options struct
struct SK_API RecorderOptions final {
    sk_sp<ImageProvider> fImageProvider;
    static constexpr int kMaxKeyContextHelpers = 2; // Leaks internal capacity!
};
```

Good:

```cpp
// include/core/SkColorSpace.h: omit fixed parameters and document preconditions
/** Callers must ensure matrix_coefficients is 0 and full_range_flag is 1. */
static sk_sp<SkColorSpace> MakeCICP(SkNamedPrimaries::CicpId color_primaries,
                                    SkNamedTransferFn::CicpId transfer_characteristics);

// include/encode/SkPngRustEncoder.h: omit unimplemented fields until supported
struct Options {
    sk_sp<SkDataTable> fComments;
};

// include/gpu/graphite/ContextOptions.h: obsolete option removed in tandem
struct ContextOptions {
    // Obsolete fVulkanVMALargeHeapBlockSize removed with legacy VMA behavior
};

// include/gpu/graphite/Recorder.h: internal constant encapsulated in class
struct SK_API RecorderOptions final {
    sk_sp<ImageProvider> fImageProvider;
};

class SK_API Recorder final : public SkRecorder {
private:
    static constexpr int kMaxKeyAndDataBuilders = 2;
};
```

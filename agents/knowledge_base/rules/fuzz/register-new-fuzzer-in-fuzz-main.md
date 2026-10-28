---
name: skia/fuzz/register-new-fuzzer-in-fuzz-main
description: When adding a fuzzer target or codec dependency in fuzz/, register it in FuzzMain.cpp, update g_type_message help, and conditionalize codec helpers
rubric: correctness
severity: should_fix
---

**Rule.** When adding a new fuzzer target (via DEF_FUZZ or in fuzz/oss_fuzz/),
register it in fuzz/FuzzMain.cpp by adding forward declarations, a wrapper
function, CLI dispatch in fuzz_file(), its type name in g_type_message (in
alphabetical order), and entries in cf_api_map. Additionally, when adding or
modifying codec fuzzer targets, ensure shared helper utilities in FuzzMain.cpp
(such as dump_png) use matching conditional compilation guards (e.g.,
SK_CODEC_ENCODES_PNG_WITH_*).

**Why.** The standalone fuzz CLI executable driven by FuzzMain.cpp and
ClusterFuzz/OSS-Fuzz reproduce crashes by resolving fuzzer names via CLI --type
flags, g_type_message help output, or cf_api_map; omitting registration or help
strings leaves targets unexecutable or undiscoverable. Furthermore, hardcoded
codec calls in FuzzMain.cpp break compilation when builds enable alternative
codec backends or disable specific codecs.

## Examples

Bad:

```cpp
// In fuzz/FuzzEncoders.cpp:
#if defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
DEF_FUZZ(PNGRustEncoder, fuzz) { ... }
#endif

// In fuzz/oss_fuzz/FuzzHdrAgtm.cpp:
void FuzzHdrAgtm(const uint8_t* data, size_t size) { ... }

// In fuzz/FuzzMain.cpp:
// Target omitted from g_type_message help and cf_api_map, and dump_png unconditionally calls libpng:
static constexpr char g_type_message[] = "How to interpret --bytes, one of:\n"
                                         "image_decode\n";
static void dump_png(const SkBitmap& bitmap) {
    if (!FLAGS_dump.isEmpty()) {
        SkFILEWStream file(FLAGS_dump[0]);
        SkPngEncoder::Encode(&file, bitmap.pixmap(), {});
    }
}
```

Good:

```cpp
// In fuzz/FuzzMain.cpp:
void FuzzHdrAgtm(const uint8_t* data, size_t size);
static void fuzz_hdr_agtm(const sk_sp<SkData>& data) {
    FuzzHdrAgtm(data->bytes(), data->size());
}

static constexpr char g_type_message[] = "How to interpret --bytes, one of:\n"
                                         "hdr_agtm\n"
                                         "image_decode\n";

static std::map<std::string, std::string> cf_api_map = {
    ...
    {"png_rust_encoder", "PNGRustEncoder"},
    {"hdr_agtm", "hdr_agtm"},
};

static void dump_png(const SkBitmap& bitmap) {
    if (!FLAGS_dump.isEmpty()) {
        SkFILEWStream file(FLAGS_dump[0]);
#if defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
        SkPngRustEncoder::Encode(&file, bitmap.pixmap(), {});
#elif defined(SK_CODEC_ENCODES_PNG_WITH_LIBPNG)
        SkPngEncoder::Encode(&file, bitmap.pixmap(), {});
#endif
    }
}
```

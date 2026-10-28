---
name: skia/factor-reusable-components-into-dedicated-targets
description: When defining Bazel build targets or filegroups, factor shared utilities, base classes, and orthogonal backend components into dedicated granular targets instead of bundling dependencies
rubric: design
severity: should_fix
---

**Rule.** Factor generic base classes, reusable utilities, and orthogonal
subsystem components (such as text shaping and unicode processing) into
dedicated, granular build targets or filegroups (e.g., skia_cc_library,
skia_filegroup) rather than bundling them into combined targets. When
implementing or depending on modular backends, depend on unbundled granular
targets (e.g., :png_codec_base, :typeface_proxy, :harfbuzz_srcs,
:skunicode_srcs) rather than combined targets that bundle multiple or unwanted
third-party dependencies.

**Why.** Bundling orthogonal components or reusable utilities into combined
targets couples independent interfaces to third-party C/C++ libraries. This
prevents consumers from independently selecting, swapping, or configuring
backends (such as pairing HarfBuzz with alternative Unicode engines), bloats
binaries, and breaks modularity.

## Examples

Bad:

```cpp
# Bad: Bundling orthogonal modular components into a combined filegroup
skia_filegroup(
    name = "harfbuzz_and_unicode_srcs",
    srcs = [
        "SkShaper_harfbuzz.cpp",
        "SkShaper_skunicode.cpp",
    ],
)

# Bad: fontmgr bundles reusable proxy with FreeType/FontConfig deps
skia_cc_library(
    name = "fontmgr_fontconfig_freetype",
    srcs = ["SkFontMgr_fontconfig.cpp", "SkTypeface_proxy.cpp"],
    deps = [":freetype_support", "//bazel/external/fontconfig"],
)
```

Good:

```cpp
# Good: Factor orthogonal dependencies into granular filegroups
skia_filegroup(
    name = "harfbuzz_srcs",
    srcs = ["SkShaper_harfbuzz.cpp"],
)
skia_filegroup(
    name = "skunicode_srcs",
    srcs = ["SkShaper_skunicode.cpp"],
)

# Good: Typeface proxy factored into standalone target without external deps
skia_cc_library(
    name = "typeface_proxy",
    srcs = ["SkTypeface_proxy.cpp"],
    deps = ["//:core"],
)
```

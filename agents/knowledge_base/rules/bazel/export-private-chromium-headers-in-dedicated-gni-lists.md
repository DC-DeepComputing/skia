---
name: skia/bazel/export-private-chromium-headers-in-dedicated-gni-lists
description: When exporting private Chromium headers to GN in bazel/exporter_tool/main.go, declare dedicated GNI file group variables rather than bundling them into shared internal source lists
rubric: design
severity: should_fix
---

**Rule.** Export headers under include/private/chromium/ into their own
dedicated GNI file group variable in bazel/exporter_tool/main.go rather than
bundling them into shared internal Skia source lists. Regenerate the
corresponding gn/*.gni files so embedders can include Chromium-specific
interfaces independently.

**Why.** Chromium consumes Skia by importing generated gn/*.gni file lists into
its GN build. If Chromium-facing private headers are bundled into generic
internal source lists, Chromium targets cannot depend on or include those
headers selectively without pulling in internal implementation files or
triggering gn check header dependency and visibility errors.

## Examples

Bad:

```cpp
// In bazel/exporter_tool/main.go:
{Var: "skia_codec_shared",
    Rules: []string{
        "//src/codec:any_decoder",
        "//include/codec:any_codec_hdrs",
        "//include/private/chromium:codec_icc_chromium_hdrs",
    },
},
```

Good:

```cpp
// In bazel/exporter_tool/main.go:
{Var: "skia_codec_shared",
    Rules: []string{
        "//src/codec:any_decoder",
        "//include/codec:any_codec_hdrs",
    },
},
{Var: "skia_codec_icc_chromium",
    Rules: []string{
        "//include/private/chromium:codec_icc_chromium_hdrs",
    },
},
```

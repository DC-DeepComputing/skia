---
name: skia/gn/stage-exported-file-list-refactorings
description: When refactoring exported GN source lists or exporter tool definitions for downstream embedders, retain legacy entries and annotate them with an attributed TODO
rubric: readability
severity: blocking
---

**Rule.** When refactoring, splitting, or renaming GN file list variables
(*.gni) or exporter tool definitions (bazel/exporter_tool/) consumed by
downstream embedders (such as Chromium, Android, or Flutter), do not remove
files or delete legacy list variables in the initial CL. Introduce new granular
lists alongside legacy entries (or retain legacy lists as aliases/aggregates),
and annotate the temporary legacy entries with an attributed tracking
TODO(issue) describing what to delete once downstream clients update. Remove the
deprecated lists only in a subsequent cleanup CL after downstream embedders land
support.

**Why.** Downstream projects directly import Skia's exported .gni files into
their own build systems (such as Chromium's //third_party/skia/BUILD.gn).
Removing files or variables before downstream build targets update causes
immediate build breakages on Skia rolls, while omitting tracking TODOs on
retained legacy entries causes temporary migration debt to be forgotten.

## Examples

Bad:

```cpp
# gn/core.gni: Immediately removing sources from an exported list in Skia:
- skia_encode_png_srcs = [ SkPngEncoderBase.cpp, SkPngEncoderImpl.cpp ]
+ skia_encode_png_srcs = [ SkPngEncoderImpl.cpp ]

// bazel/exporter_tool/main.go: Retaining compatibility rules without a TODO:
{Var: "skia_codec_rust_png_public",
	Rules: []string{
		"//experimental/rust_png:hdrs",
		"//experimental/rust_png/decoder:hdrs",
	},
},
```

Good:

```cpp
# gn/core.gni: Introduce granular lists and retain legacy lists with a TODO:
skia_encode_png_base = [ SkPngEncoderBase.cpp ]
# TODO(https://crbug.com/381900683): Remove once Chromium references skia_encode_png_base.
skia_encode_png_srcs = [ SkPngEncoderBase.cpp, SkPngEncoderImpl.cpp ]

// bazel/exporter_tool/main.go: Retain legacy targets with an attributed TODO:
{Var: "skia_codec_rust_png_public",
	Rules: []string{
		// TODO(https://crbug.com/379312510): Delete `rust_png:hdrs` once Chromium updates.
		"//experimental/rust_png:hdrs",
		"//experimental/rust_png/decoder:hdrs",
	},
},
```

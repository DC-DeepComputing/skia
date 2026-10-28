---
name: skia/bazel/suffix-export-source-lists-with-srcs
description: When defining GNI file list export variables in Bazel exporter configurations, suffix implementation source lists with _srcs rather than bare component names
rubric: readability
severity: nit
---

**Rule.** When defining build file list variables that export implementation
sources in bazel/exporter_tool/main.go (or gn/*.gni), suffix the variable name
with _srcs (or _sources) rather than using a bare component name. Reserve bare
component names or _public suffixes for public header lists, ensuring source and
header lists remain clearly distinguished.

**Why.** Using an unadorned component name (such as skia_codec_libpng) for an
internal source list is ambiguous and leaves no clean convention when public
header lists (_public) or separate interface targets are added later. Suffixing
with _srcs makes list contents explicit and future-proofs export mappings.

## Examples

Bad:

```cpp
{Var: "skia_codec_libpng",
	Rules: []string{
		"//src/codec:buffet_libpng_srcs",
		"//src/codec:common_libpng_srcs",
	},
}
```

Good:

```cpp
{Var: "skia_codec_libpng_srcs",
	Rules: []string{
		"//src/codec:buffet_libpng_srcs",
		"//src/codec:common_libpng_srcs",
	},
}
```

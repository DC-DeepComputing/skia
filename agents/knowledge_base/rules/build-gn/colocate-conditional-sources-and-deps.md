---
name: skia/build-gn/colocate-conditional-sources-and-deps
description: When conditionally adding source files in GN build files, co-locate required dependencies in the same conditional block instead of duplicating them across downstream branches
rubric: design
severity: should_fix
---

**Rule.** When conditionally adding source files (`sources += ...`) to a GN
build target, declare their required dependencies (`deps += ...`) within the
same conditional block. Do not separate or duplicate dependency additions across
multiple downstream conditional branches.

**Why.** Target dependencies belong directly with the source files that require
them. Co-locating them in the same conditional block eliminates redundant
configuration checks, keeps build configuration modular, and prevents
dependencies from being missed or orphaned when build conditions change.

## Examples

Bad:

```cpp
} else if (skia_use_client_icu || skia_use_libgrapheme) {
  sources += skia_unicode_icu_bidi_sources
}

if (skia_use_client_icu) {
  sources += skia_unicode_client_icu_sources
  if (!skia_use_icu) {
    deps += [ skia_icu_bidi_third_party_dir ]
  }
}
if (skia_use_libgrapheme) {
  sources += skia_unicode_libgrapheme_sources
  deps += [ skia_libgrapheme_third_party_dir ]
  if (!skia_use_icu) {
    deps += [ skia_icu_bidi_third_party_dir ]
  }
}
```

Good:

```cpp
} else if (skia_use_client_icu || skia_use_libgrapheme) {
  sources += skia_unicode_icu_bidi_sources
  deps += [ skia_icu_bidi_third_party_dir ]
}

if (skia_use_client_icu) {
  sources += skia_unicode_client_icu_sources
}
if (skia_use_libgrapheme) {
  sources += skia_unicode_libgrapheme_sources
  deps += [ skia_libgrapheme_third_party_dir ]
}
```

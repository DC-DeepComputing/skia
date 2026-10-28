---
name: skia/build-gn/avoid-optional-for-internal-helpers
description: When defining internal helper targets in GN build files, use unconditional skia_source_set instead of wrapping them in optional() with borrowed feature flags
rubric: design
severity: should_fix
---

**Rule.** In GN build files, do not wrap internal helper source sets or utility
targets in the optional(...) template or tie them to borrowed or temporary
feature flags. Define internal helpers unconditionally as skia_source_set
targets and have consuming targets depend on them directly via deps.

**Why.** Skia's optional(...) GN template is intended for features requiring
third-party libraries or public feature-flag gating. Wrapping internal helper
targets in optional() with borrowed feature flags unnecessarily couples the
helper to unrelated configurations and disables it for other consumers when that
flag is off.

## Examples

Bad:

```cpp
optional("typeface_proxy") {
  enabled = skia_enable_fontmgr_fontconfig # for now
  sources = [
    "src/ports/SkTypeface_proxy.cpp",
    "src/ports/SkTypeface_proxy.h",
  ]
}
```

Good:

```cpp
skia_source_set("typeface_proxy") {
  sources = [
    "src/ports/SkTypeface_proxy.cpp",
    "src/ports/SkTypeface_proxy.h",
  ]
}

optional("fontmgr_fontconfig") {
  enabled = skia_enable_fontmgr_fontconfig
  deps = [ ":typeface_proxy" ]
}
```

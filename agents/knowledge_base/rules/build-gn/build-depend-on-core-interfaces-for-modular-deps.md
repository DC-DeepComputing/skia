---
name: skia/build-gn/build-depend-on-core-interfaces-for-modular-deps
description: When configuring modular build targets, depend on core interface targets and do not gate modules on concrete backends or optional dependencies
rubric: correctness
severity: should_fix
---

**Rule.** Do not make high-level module build targets (such as skparagraph,
skottie, or svg) depend directly on concrete backend implementations of
configurable dependencies (such as HarfBuzz, ICU, or specific codecs), and do
not gate module builds on internal third-party flags (e.g., skia_use_icu) or on
optional module dependencies whose usage is guarded by preprocessor macros
(e.g., SK_UNICODE_AVAILABLE). Instead, depend only on core interface targets
(e.g., skshaper_core, skunicode_core), and guard module targets strictly using
their own module enablement flags (e.g., skia_enable_<module> imported from
<module>.gni) and mandatory dependencies.

**Why.** Hardcoding concrete backend implementations in target dependencies or
gating modules on internal third-party flags or optional dependencies breaks
Skia's modular architecture and build layering. It couples targets to specific
providers, prevents embedders from substituting alternative backends (such as
ICU4X or libgrapheme for Unicode), and disables consuming modules even when
valid alternative implementations exist or when the module compiles cleanly
without the optional component.

## Examples

Bad:

```cpp
# Bad: Gating module target on internal third-party flags
import("../skshaper/skshaper.gni")

if (skia_enable_skparagraph && (skia_use_icu || skia_use_client_icu) &&
    skia_enable_skshaper) {
  ...
}

# Bad: Requiring optional module dependencies guarded by preprocessor macros
import("../skunicode/skunicode.gni")
import("../../gn/skia.gni")

if (skia_enable_skottie && skia_enable_skunicode) {
  ...
}

# Bad: Hardcoding concrete HarfBuzz and ICU backends in modular target deps
skia_cc_library(
    name = "skparagraph",
    deps = [
        "//modules/skshaper:skshaper_harfbuzz",
        "//modules/skunicode:skunicode_icu",
    ],
)
```

Good:

```cpp
# Good: Gating on module enablement flags for mandatory dependencies
import("../skshaper/skshaper.gni")
import("../skunicode/skunicode.gni")

if (skia_enable_skparagraph && skia_enable_skshaper && skia_enable_skunicode) {
  ...
}

# Good: Gating only on module's own flag when dependency is optional (#ifdef SK_UNICODE_AVAILABLE)
import("../../gn/skia.gni")

if (skia_enable_skottie) {
  ...
}

# Good: Depending on core interface targets; clients link concrete backends
skia_cc_library(
    name = "skparagraph",
    deps = [
        "//:core",
        "//modules/skshaper:skshaper_core",
        "//modules/skunicode:skunicode_core",
    ],
)
```

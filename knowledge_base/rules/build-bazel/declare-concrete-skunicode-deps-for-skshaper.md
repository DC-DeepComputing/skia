---
name: skia/build-bazel/declare-concrete-skunicode-deps-for-skshaper
description: When configuring Bazel targets with modular skshaper backends, explicitly declare dependencies on concrete SkUnicode targets rather than relying on transitive inclusion
rubric: design
severity: blocking
---

**Rule.** When configuring Bazel build targets to use modular skshaper
implementations (such as skshaper_unicode or skshaper_harfbuzz), explicitly
declare a dependency on the required concrete SkUnicode backend target (e.g.,
//:skunicode_icu or //modules/skunicode:skunicode_icu) in deps. Do not assume
modular shaper targets will transitively pull in a concrete Unicode
implementation.

**Why.** skshaper targets like //:skshaper_unicode intentionally depend only on
the abstract //modules/skunicode:skunicode_core interface to keep text shaping
decoupled from specific Unicode libraries. Targets configuring modular shaping
must explicitly link their chosen concrete SkUnicode backend, or symbol
resolution and shaper factory initialization will fail.

## Examples

Bad:

```cpp
skia_cc_library(
    name = "client_lib",
    deps = [
        "//:skshaper_harfbuzz",
        "//:skshaper_unicode",
        "//modules/skottie",
    ],
)
```

Good:

```cpp
skia_cc_library(
    name = "client_lib",
    deps = [
        "//:skshaper_harfbuzz",
        "//:skshaper_unicode",
        "//:skunicode_icu",
        "//modules/skottie",
    ],
)
```

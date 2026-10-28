---
name: skia/gn/android-bp-declare-cxx-bridge-deps
description: When configuring Android Blueprint library targets in gn/gn_to_bp.py that enable Rust or Fontations, declare generated CXX bridge headers and sources
rubric: correctness
severity: blocking
---

**Rule.** When enabling Fontations or Rust CXX bridge dependencies in Android
Blueprint library targets (such as libskia_renderengine in gn/gn_to_bp.py),
explicitly declare generated bridge dependencies in both generated_headers and
generated_sources. Ensure targets compiling translation units that include CXX
bridge headers (such as ffi.rs.h) depend on libskia_cxx_bridge_header and
libskia_cxx_bridge_code.

**Why.** Android's Soong build system does not automatically expose generated
CXX bridge headers and object files to static library targets. Omitting
generated_headers and generated_sources causes compilation failures in
translation units that include bridge headers during Android platform tree
builds.

## Examples

Bad:

```cpp
# gn/gn_to_bp.py
cc_library_static {
    name: "libskia_renderengine",
    defaults: ["skia_defaults",
               "skia_renderengine_deps"],
    srcs: [
        $renderengine_srcs
    ],
    export_include_dirs: [
        "renderengine",
    ],
}
```

Good:

```cpp
# gn/gn_to_bp.py
cc_library_static {
    name: "libskia_renderengine",
    defaults: ["skia_defaults",
               "skia_renderengine_deps"],
    srcs: [
        $renderengine_srcs
    ],
    export_include_dirs: [
        "renderengine",
    ],
    generated_headers: [
        "libskia_cxx_bridge_header",
    ],
    generated_sources: [
        "libskia_cxx_bridge_code",
    ],
}
```

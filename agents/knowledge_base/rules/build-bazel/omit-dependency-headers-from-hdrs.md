---
name: skia/build-bazel/omit-dependency-headers-from-hdrs
description: When configuring Bazel library hdrs, do not re-export headers available via deps, but declare header filegroups directly in hdrs to avoid circular dependencies
rubric: design
severity: should_fix
---

**Rule.** When defining Bazel C++ library targets (skia_cc_library or
cc_library), do not include headers from dependency targets in hdrs when they
are already available via deps and are not part of the target's public API.
Conversely, when depending on the providing library target in deps would
introduce a circular dependency (e.g., an unbundled helper target consumed by
the library whose headers it needs), declare the required header filegroup
directly in hdrs to satisfy compiler layering checks without creating a cycle.

**Why.** In Bazel, hdrs declares the public interface exposed transitively to
downstream dependents, while deps supplies headers needed for compilation.
Re-exporting available dependency headers leaks internal details and creates
brittle transitive dependencies. However, when a circular dependency prevents
adding the parent library to deps, directly referencing its header filegroup in
hdrs satisfies layering_check while keeping the dependency graph acyclic.

## Examples

Bad:

```cpp
# Anti-pattern 1: Re-exporting dependency headers already available via deps
skia_cc_library(
    name = "fontmgr_android_ndk",
    hdrs = ["//include/ports:android_ndk_hdrs", "//include/ports:typeface_hdrs"],
    deps = [":freetype_support"],  # typeface_hdrs already provided here
)
# Anti-pattern 2: Omitting included headers to avoid circular deps, failing layering_check
skia_cc_library(
    name = "jpeg_helpers",
    hdrs = ["//include/docs:pdf_jpeg_hdrs"],  # Omits pdf_hdrs because :pdf depends on :jpeg_helpers
    deps = ["//:jpeg_decode_codec"],
)
```

Good:

```cpp
# Pattern 1: Reserve hdrs for target interface; consume dependencies via deps
skia_cc_library(
    name = "fontmgr_android_ndk",
    hdrs = ["//include/ports:android_ndk_hdrs"],
    deps = [":freetype_support"],
)
# Pattern 2: Include header filegroup in hdrs when depending on target would create a cycle
skia_cc_library(
    name = "jpeg_helpers",
    hdrs = ["//include/docs:pdf_hdrs", "//include/docs:pdf_jpeg_hdrs"],
    features = ["layering_check"],
    deps = ["//:jpeg_decode_codec"],
)
```

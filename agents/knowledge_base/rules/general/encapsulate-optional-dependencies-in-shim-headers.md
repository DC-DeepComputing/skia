---
name: skia/encapsulate-optional-dependencies-in-shim-headers
description: When integrating optional or embedder-varying dependencies in internal code, encapsulate them behind repo-relative shim headers instead of Bazel includes or GN include_dirs
rubric: design
severity: should_fix
---

**Rule.** Do not use Bazel `includes` or GN `include_dirs` in internal source
directories (`src/**`, `include/private/**`) to expose subdirectories as root
include paths. Encapsulate optional or embedder-varying third-party dependencies
behind repository-relative shim headers (such as
`src/partition_alloc/raw_ptr.h`) that conditionally `#include` either the real
dependency or a local fallback, keeping preprocessor guards and custom include
search paths hidden from calling code.

**Why.** Skia's internal source files, downstream roll scripts (e.g. Google3),
and embedder integrations rely on consistent repo-relative include paths
(`src/...` or `include/...`). Exposing internal subdirectories as root include
paths via build directives pollutes search paths and breaks builds across
embedders where dependency layouts differ.

## Examples

Bad:

```cpp
# src/partition_alloc/BUILD.bazel
skia_cc_library(
    name = "raw_ptr_noop",
    hdrs = ["partition_alloc/pointers/raw_ptr.h"],
    includes = ["."],
)

// src/sksl/SkSLParser.cpp
#include "partition_alloc/pointers/raw_ptr.h"
```

Good:

```cpp
# src/partition_alloc/BUILD.bazel
skia_cc_library(
    name = "raw_ptr",
    srcs = ["noop/raw_ptr.h"],
    hdrs = ["raw_ptr.h"],
    visibility = ["//:__subpackages__"],
)

// src/partition_alloc/raw_ptr.h
#if defined(SK_USE_PARTITION_ALLOC)
#include "partition_alloc/pointers/raw_ptr.h"
#else
#include "src/partition_alloc/noop/raw_ptr.h"
#endif

// src/sksl/SkSLParser.cpp
#include "src/partition_alloc/raw_ptr.h"
```

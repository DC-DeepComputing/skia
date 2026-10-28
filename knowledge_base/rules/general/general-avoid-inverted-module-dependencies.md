---
name: skia/general-avoid-inverted-module-dependencies
description: When authoring lower-level or platform code, do not include headers from optional feature modules; place shared interface headers in the lower-level subsystem
rubric: design
severity: should_fix
---

**Rule.** Do not include headers from higher-level or optional feature modules
(such as src/pdf/, src/xps/, src/svg/, or modules/) in lower-level, core, or
platform modules (such as src/ports/, src/core/, or src/base/). When a shared
interface or utility is implemented by platform code or consumed across
subsystems, define and locate its header in the implementing or lower-level
subsystem rather than inside an optional feature backend.

**Why.** Skia enforces strict architectural layering where core and platform
subsystems must remain decoupled from optional feature backends and document
generators. Inverted dependencies cause circular build graph edges, prevent
compiling lower-level subsystems when optional features are disabled, and
violate modular library encapsulation.

## Examples

Bad:

```cpp
// In src/ports/SkJpegInfo_libjpegturbo.cpp or SkJpegInfo_none.cpp:
#include "src/pdf/SkJpegInfo.h"

bool SkGetJpegInfo(const void* data, size_t len, SkJpegInfo* info) {
    // ...
}
```

Good:

```cpp
// In src/ports/SkJpegInfo_libjpegturbo.cpp or SkJpegInfo_none.cpp:
#include "src/ports/SkJpegInfo.h"

bool SkGetJpegInfo(const void* data, size_t len, SkJpegInfo* info) {
    // ...
}
```

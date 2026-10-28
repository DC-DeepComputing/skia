---
name: skia/api/include-std-headers-for-inline-methods
description: When defining inline methods in exported headers that call standard library utilities, explicitly include required standard headers instead of relying on transitive includes
rubric: readability
severity: blocking
---

**Rule.** Ensure exported header files are self-contained by explicitly
including required standard library headers (such as <algorithm> for
std::clamp/std::min/std::max, <cmath> for std::round/std::floor, or <utility>
for std::move/std::forward) used by inline method definitions. Do not rely on
transitive includes through other headers.

**Why.** Transitive header inclusions vary across standard library
implementations (libc++, libstdc++, MSVC STL) and downstream embedders like
Chromium and Android. While Skia style prefers minimizing includes via forward
declaration, inline methods calling standard library templates cannot
forward-declare them and break downstream embedder builds if required headers
are omitted.

## Examples

Bad:

```cpp
// include/private/SkHdrMetadata.h (missing <algorithm> and <cmath>)
uint16_t getUint16MaxCLL() const {
    return static_cast<uint16_t>(std::clamp(std::round(fMaxCLL), 0.f, 65535.f));
}
```

Good:

```cpp
// include/private/SkHdrMetadata.h
#include <algorithm>
#include <cmath>

uint16_t getUint16MaxCLL() const {
    return static_cast<uint16_t>(std::clamp(std::round(fMaxCLL), 0.f, 65535.f));
}
```

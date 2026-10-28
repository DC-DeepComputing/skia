---
name: skia/prefer-skstring-over-stringstream
description: When formatting or building strings, avoid C++ string streams like std::stringstream or std::ostringstream; use SkString and SkStringPrintf instead
rubric: readability
severity: should_fix
---

**Rule.** Do not use C++ string streams (std::stringstream, std::ostringstream,
std::istringstream, or <sstream>) for string formatting or construction; use
Skia's SkString and SkStringPrintf (or SkWStream) instead.

**Why.** Standard library iostreams and string streams introduce persistent
formatting state, heavy template instantiations, and binary size bloat. Skia
standardizes on SkStringPrintf and SkString (or SkDynamicMemoryWStream for
streaming data) to keep string formatting lightweight and consistent across
platforms.

## Examples

Bad:

```cpp
#include <sstream>

std::string formatShader(int sampleCount) {
    std::ostringstream ss;
    ss << "@group(0) @binding(0) var colorMap: texture_multisampled_2d<f32>;\n";
    ss << "const sampleCount = " << sampleCount << ";\n";
    return ss.str();
}
```

Good:

```cpp
#include "include/core/SkString.h"

SkString formatShader(int sampleCount) {
    static constexpr char kShaderTemplate[] =
        "@group(0) @binding(0) var colorMap: texture_multisampled_2d<f32>;\n"
        "const sampleCount = %d;\n";
    return SkStringPrintf(kShaderTemplate, sampleCount);
}
```

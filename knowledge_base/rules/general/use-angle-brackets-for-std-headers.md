---
name: skia/use-angle-brackets-for-std-headers
description: When organizing imports and declarations in C++ files, place quoted headers first, system and third-party headers second, and separate blocks with blank lines
rubric: readability
severity: should_fix
---

**Rule.** Organize C++ file imports and declarations into distinct,
alphabetically sorted blocks separated by blank lines: quoted Skia project
headers ("...") first, third-party and standard library headers (<...>) second,
and forward declarations third. Keep headers from the same library grouped
together and sorted alphabetically; do not scatter third-party headers across
standard library includes. Always separate the #include section from subsequent
forward declarations, code declarations, or function definitions (such as
DEF_TEST macros) with an empty line. Do not place standard headers above Skia
headers, use double quotes for standard headers, or omit separating blank lines
between sections or before code.

**Why.** Placing Skia project headers before standard library and third-party
includes ensures internal configuration and macros take precedence and prevents
embedder include collisions. Grouping related third-party and standard headers
into distinct alphabetically sorted blocks and separating them from forward
declarations and code declarations with blank lines visually demarcates imports
from logic, keeps dependencies clear, and minimizes merge conflicts.

## Examples

Bad:

```cpp
// Missing blank lines between blocks, standard before local, or code directly after include:
#include <optional>
#include "include/core/SkColorSpace.h"
#include "string"
#include "tools/ToolUtils.h"
class SkData;

// Scattered third-party and unsorted standard headers:
#include <ICU4XBidi.hpp>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <ICU4XCaseMapper.hpp>

#include <cmath>
DEF_TEST(HairlineClosedContourCapOffset_Draw, reporter) {
    ...
}
```

Good:

```cpp
// Quoted Skia headers first:
#include "include/core/SkColorSpace.h"
#include "tools/ToolUtils.h"

// Third-party library headers grouped and sorted alphabetically:
#include <ICU4XBidi.hpp>
#include <ICU4XCaseMapper.hpp>

// Standard library headers grouped and sorted alphabetically:
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// Forward declarations separated by blank lines:
class SkData;

// Empty line between includes and subsequent code declarations:
DEF_TEST(HairlineClosedContourCapOffset_Draw, reporter) {
    ...
}
```

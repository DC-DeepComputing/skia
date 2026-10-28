---
name: skia/prefer-cpp-standard-library-headers
description: When including C standard library facilities in C++ files, prefer C++ wrapper headers like <cstdarg> or <cinttypes> over legacy <... .h> headers
rubric: readability
severity: should_fix
---

**Rule.** In C++ source and header files, include C standard library facilities
using modern C++ <c...> wrapper headers (such as <cstdarg>, <cstdio>, <cmath>,
<cstdint>, <cinttypes>) rather than legacy C <... .h> headers (such as
<stdarg.h>, <stdio.h>, <math.h>, <inttypes.h>). Keep all standard library
headers sorted alphabetically within the angle-bracketed include section. In
particular, include <cinttypes> directly for format macros (such as PRId64)
without defining __STDC_FORMAT_MACROS.

**Why.** C++ <c...> wrapper headers integrate standard C library facilities into
C++ compilation units, declare symbols portably in namespace std, and prevent
legacy C header pollution while keeping includes cleanly sorted. Standard C++
headers such as <cinttypes> unconditionally provide format macros without
requiring obsolete C99/C++98 __STDC_FORMAT_MACROS preprocessor definitions.

## Examples

Bad:

```cpp
// Legacy C headers and unsorted includes
#include <sstream>
#include <stdarg.h>
#include <string>

// Obsolete macro definition workaround for PRI* format macros
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
```

Good:

```cpp
// C++ standard library wrappers sorted alphabetically
#include <cinttypes>
#include <cstdarg>
#include <sstream>
#include <string>
```

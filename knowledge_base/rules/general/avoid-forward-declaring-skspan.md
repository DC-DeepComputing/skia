---
name: skia/avoid-forward-declaring-skspan
description: When referencing SkSpan in header files, include include/core/SkSpan.h directly instead of forward-declaring template <typename T> class SkSpan
rubric: readability
severity: should_fix
---

**Rule.** Do not forward-declare template <typename T> class SkSpan; in header
files; include "include/core/SkSpan.h" directly instead.

**Why.** SkSpan is a lightweight vocabulary type that is conditionally defined
as an alias template (template <typename T> using SkSpan = std::span<T>;). In
C++, alias templates cannot be forward-declared as class templates, causing
compilation failures when transitioning to std::span or when legacy SkSpan mode
is disabled.

## Examples

Bad:

```cpp
// Header file: forward-declaring SkSpan as a template class
template <typename T> class SkSpan;

class Codec {
public:
    void decode(SkSpan<uint8_t> dst);
};
```

Good:

```cpp
// Header file: directly include the lightweight SkSpan header
#include "include/core/SkSpan.h"

class Codec {
public:
    void decode(SkSpan<uint8_t> dst);
};
```

---
name: skia/guard-floating-point-from-chars
description: 'When parsing floating-point numbers with std::from_chars, guard with #if defined(__cpp_lib_to_chars) and fall back to SkAutoLocaleSetter with strtof or strtod for toolchain compatibility'
rubric: correctness
severity: blocking
---

**Rule.** Do not use floating-point std::from_chars unconditionally without #if
defined(__cpp_lib_to_chars). On toolchains lacking floating-point <charconv>
support, provide a fallback using SkAutoLocaleSetter("C") paired with standard C
parsing routines (strtof, strtod) to guarantee locale-independent parsing across
all supported compilers.

**Why.** While C++17 introduced <charconv>, floating-point overloads of
std::from_chars were omitted from several standard library implementations (such
as earlier LLVM libc++ toolchains). Unconditional use causes compilation
failures on those toolchains, whereas standard C parsing functions like strtof
depend on the current locale unless wrapped with SkAutoLocaleSetter.

## Examples

Bad:

```cpp
// Incomplete toolchain support for floating-point std::from_chars
#include <charconv>

float f;
auto result = std::from_chars(p, p_stop + 1, f);
if (result.ec == std::errc{}) {
    this->pushFloat(f);
    return result.ptr;
}
```

Good:

```cpp
#include <charconv>
#include "src/core/SkAutoLocaleSetter.h"

#if defined(__cpp_lib_to_chars)
float f;
auto result = std::from_chars(p, p_stop + 1, f);
if (result.ec == std::errc{}) {
    this->pushFloat(f);
    return result.ptr;
}
#else
SkAutoLocaleSetter locale("C");
char* matched;
float f = strtof(p, &matched);
if (matched > p) {
    this->pushFloat(f);
    return matched;
}
#endif
```

---
name: skia/prefer-documented-architecture-intrinsics
description: When invoking architecture-specific operations, use documented intrinsics from standard architecture headers (such as <x86intrin.h>) instead of compiler-internal builtins like __builtin_ia32_*
rubric: readability
severity: should_fix
---

**Rule.** Use documented architecture intrinsics declared in standard
architecture headers (such as <x86intrin.h>, <immintrin.h>, or <arm_neon.h>)
rather than undocumented compiler-internal builtins (such as __builtin_ia32_*).
Ensure the appropriate intrinsic header is included to provide portable and
stable interfaces across supported compilers.

**Why.** Compiler-internal builtins like __builtin_ia32_* are undocumented
toolchain implementation details that are non-portable across compilers and lack
stability guarantees across compiler versions. Standard architecture intrinsic
headers provide documented, portable, and supported interfaces across Clang,
GCC, and MSVC.

## Examples

Bad:

```cpp
#if defined(__x86_64__)
// Bad: relying on undocumented compiler-internal builtin
uint32_t cpuInfo;
return __builtin_ia32_rdtscp(&cpuInfo);
#endif
```

Good:

```cpp
#if defined(__x86_64__)
// Good: include standard architecture header and call documented intrinsic
#include <x86intrin.h>
unsigned aux;
return __rdtscp(&aux);
#endif
```

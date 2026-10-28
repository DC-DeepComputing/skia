---
name: skia/prefer-std-sort-over-bespoke-sort
description: When sorting arrays or element ranges, prefer std::sort over bespoke sort routines or legacy SkTQSort for superior compiler and integer optimizations
rubric: safety
severity: should_fix
---

**Rule.** Prefer standard library std::sort (or std::stable_sort when relative
order must be preserved) over bespoke sorting routines and legacy helpers like
SkTQSort or SkTSort. When replacing legacy sorting or writing new sorting logic,
use std::sort(begin, end) with <algorithm>.

**Why.** Standard library sorting implementations are heavily tuned and leverage
compiler optimizations—such as specialized block quicksort implementations for
32-bit and 64-bit primitive types—that significantly outperform bespoke sorting
algorithms and legacy codebase helpers.

## Examples

Bad:

```cpp
#include "src/core/SkTSort.h"

// Using legacy SkTQSort for sorting elements
SkTQSort<int>(array, array + N);
```

Good:

```cpp
#include <algorithm>

// Using std::sort for optimized standard library sorting
std::sort(array, array + N);
```

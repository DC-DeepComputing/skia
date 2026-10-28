---
name: skia/core/remove-legacy-utility-headers-during-migration
description: When replacing bespoke utility algorithms like SkTSort with standard library equivalents, remove the legacy header completely to avoid binary size bloat from duplicate instantiations
rubric: safety
severity: should_fix
---

**Rule.** When replacing legacy bespoke utility implementations or algorithms
(such as SkTSort.h) with standard library equivalents (such as std::sort),
migrate all call sites and remove the legacy header completely within the
refactoring. Avoid partial migrations that leave legacy utility functions
alongside new standard library instantiations.

**Why.** Retaining bespoke utility functions while introducing standard library
equivalents compiles both implementations into the final binary. For templated
algorithms, duplicate instantiations cause measurable binary size bloat (such as
+10 KB) that can be eliminated by fully deprecating and removing the legacy
utility.

## Examples

Bad:

```cpp
// Migrating some call sites to std::sort while retaining legacy SkTSort:
#include <algorithm>
#include "src/core/SkTSort.h"

std::sort(span.begin(), span.end());
...
// Retaining legacy functions leaves duplicate template instantiations in binary:
SkTQSort(pts.begin(), pts.end());
```

Good:

```cpp
// Fully migrate callers to standard algorithms and remove the legacy header:
#include <algorithm>

std::sort(span.begin(), span.end());
std::sort(pts.begin(), pts.end());
// Remove src/core/SkTSort.h completely to eliminate duplicate template bloat.
```

---
name: skia/keep-includes-outside-namespaces
description: 'When adding or modifying #include directives in C++ files, place all includes at global file scope outside any namespace blocks'
rubric: readability
severity: should_fix
---

**Rule.** Do not place `#include` directives inside `namespace` blocks. Place
all `#include` directives at the top of the file at global file scope, outside
any namespace declarations.

**Why.** Placing an `#include` directive inside a namespace block causes all
types, functions, and symbols declared in the included header to be scoped
inside that namespace rather than at global or intended scope. Even if header
guards appear to mask compilation errors when the header is transitively
included earlier, it creates brittle, order-dependent include dependencies that
break unexpectedly when include orders change.

## Examples

Bad:

```cpp
namespace SkSL {
namespace RP {

#if !defined(SKSL_STANDALONE)
#include "src/core/SkRasterPipeline.h"
#endif

void Program::appendStages(SkRasterPipeline* pipeline, ...) {
```

Good:

```cpp
#if !defined(SKSL_STANDALONE)
#include "src/core/SkRasterPipeline.h"
#endif

namespace SkSL {
namespace RP {

void Program::appendStages(SkRasterPipeline* pipeline, ...) {
```

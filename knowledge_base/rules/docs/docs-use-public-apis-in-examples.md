---
name: skia/docs/docs-use-public-apis-in-examples
description: When writing or modifying documentation and Fiddle examples in docs/examples/, use only public Skia APIs instead of internal macros, private headers, or SkAssertResult
rubric: readability
severity: should_fix
---

**Rule.** Do not use private Skia APIs, internal macros (such as SkAssertResult
or SkASSERT), or headers from include/private/ and src/ in public documentation
and Fiddle examples under docs/examples/. Write example code using exclusively
public client-facing APIs and standard control flow or assertions.

**Why.** Examples under docs/examples/ are compiled for Fiddle and published as
public documentation for external developers. Using internal macros like
SkAssertResult or including private headers leaks internal implementation
details and causes build failures when external clients copy and paste example
snippets into downstream projects.

## Examples

Bad:

```cpp
#include "include/private/base/SkAssert.h"

void draw(SkCanvas* canvas) {
    SkPixmap pm;
    SkAssertResult(image->makeRasterImage()->peekPixels(&pm));
}
```

Good:

```cpp
void draw(SkCanvas* canvas) {
    SkPixmap pm;
    if (image->makeRasterImage()->peekPixels(&pm)) {
        // Use pixel data
    }
}
```

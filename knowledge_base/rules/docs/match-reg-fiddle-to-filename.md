---
name: skia/docs/match-reg-fiddle-to-filename
description: When defining Fiddle examples in docs/examples/, ensure the identifier passed to REG_FIDDLE, REG_FIDDLE_ANIMATED, or REG_FIDDLE_SRGB matches the filename basename
rubric: readability
severity: should_fix
---

**Rule.** When defining Fiddle examples in docs/examples/<Name>.cpp, ensure the
example identifier passed to REG_FIDDLE, REG_FIDDLE_ANIMATED, or REG_FIDDLE_SRGB
matches the file's basename (<Name>). Do not retain or duplicate an existing
example's identifier when copying or adapting an example file.

**Why.** All example files under docs/examples/ are compiled together into a
single translation unit (tools/fiddle/all_examples.cpp). Duplicating an
identifier causes compiler redefinition errors (reg_##NAME and
example_##NAME::draw) and breaks documentation @<Name> lookup embeds.

## Examples

Bad:

```cpp
// In docs/examples/SKIA_LOGO_pride.cpp:
#include "tools/fiddle/examples.h"

REG_FIDDLE(SKIA_LOGO, 816, 464, false, 0) {
void draw(SkCanvas* canvas) {
    // ...
}
}  // END FIDDLE
```

Good:

```cpp
// In docs/examples/SKIA_LOGO_pride.cpp:
#include "tools/fiddle/examples.h"

REG_FIDDLE(SKIA_LOGO_pride, 816, 464, false, 0) {
void draw(SkCanvas* canvas) {
    // ...
}
}  // END FIDDLE
```

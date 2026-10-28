---
name: skia/api/avoid-redundant-tokens-in-helper-header-filenames
description: When naming helper header files, avoid redundant format or subsystem token stuttering merely for alphabetical sorting; name them directly by subsystem and implementation
rubric: readability
severity: nit
---

**Rule.** When naming helper header files (such as
`Sk<Subsystem><Format>Helpers.h`), do not insert redundant format or subsystem
prefixes (e.g., avoid `SkXPSPngLibPngHelpers.h`) merely to group related files
alphabetically. Name them directly by combining the subsystem prefix with the
specific implementation or format (e.g., prefer `SkXPSLibpngHelpers.h` or
`SkPDFJpegHelpers.h`).

**Why.** Repeating format or subsystem identifiers in file names (such as 'Png'
in 'PngLibPng') produces redundant stuttering and clutters include paths.
Concise names that pair the subsystem directly with the implementation are
clearer, avoid duplication, and adhere to established header naming conventions
across Skia.

## Examples

Bad:

```cpp
// Redundant format prefix ('Png') added for alphabetical grouping
#include "include/docs/SkXPSPngLibPngHelpers.h"
#include "include/docs/SkXPSPngRustPngHelpers.h"
```

Good:

```cpp
// Direct naming by subsystem and implementation without stuttering
#include "include/docs/SkXPSLibpngHelpers.h"
#include "include/docs/SkXPSRustPngHelpers.h"
```

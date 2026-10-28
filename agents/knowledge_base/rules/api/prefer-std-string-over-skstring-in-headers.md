---
name: skia/api/prefer-std-string-over-skstring-in-headers
description: When declaring string-returning methods in headers that already include <string>, return std::string instead of forward-declaring or including SkString
rubric: readability
severity: nit
---

**Rule.** In header files that already `#include <string>`, prefer returning or
using `std::string` rather than forward-declaring or including `SkString`. Avoid
introducing redundant custom string dependencies or forward declarations when
standard string is already part of the header's interface.

**Why.** When a header already includes standard `<string>`, using `std::string`
avoids pulling in an unnecessary dependency or forward declaration for
`SkString`. This keeps header interfaces aligned with standard C++ vocabulary
types and reduces header coupling.

## Examples

Bad:

```cpp
#include <string>

class SkString;

struct VulkanYcbcrConversionInfo {
    SkString toString() const;
};
```

Good:

```cpp
#include <string>

struct VulkanYcbcrConversionInfo {
    std::string toString() const;
};
```

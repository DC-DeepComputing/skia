---
name: skia/rust/reuse-spanutils-for-rust-slice
description: When referencing rust::Slice in C++ FFI headers, include rust/common/SpanUtils.h instead of manually duplicating CXX bridge forward declarations
rubric: design
severity: should_fix
---

**Rule.** In C++ headers interfacing with Rust FFI, do not manually duplicate
forward declarations of CXX bridge types such as `rust::Slice` (or
`cxxbridge1::Slice`). Instead, `#include "rust/common/SpanUtils.h"`, which
provides the canonical forward declaration and slice-to-span conversion helpers
(`ToSkSpan`).

**Why.** CXX bridge types require specific inline namespace nesting
(`rust::cxxbridge1`). Re-declaring `rust::Slice` across multiple C++ headers
creates redundant boilerplate and maintenance friction if CXX internals or
header organizations change. Centralizing the declaration in
`rust/common/SpanUtils.h` ensures consistency and provides safe span conversion
utilities across the codebase.

## Examples

Bad:

```cpp
// Forward declaration of rust::Slice from CXX generated code
namespace rust {
inline namespace cxxbridge1 {
template <typename T> class Slice;
}  // namespace cxxbridge1
}  // namespace rust

class SkStreamAdapter {
public:
    size_t read(rust::Slice<uint8_t> buffer);
};
```

Good:

```cpp
#include "rust/common/SpanUtils.h"

class SkStreamAdapter {
public:
    size_t read(rust::Slice<uint8_t> buffer);
};
```

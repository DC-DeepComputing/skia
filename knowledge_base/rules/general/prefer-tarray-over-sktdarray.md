---
name: skia/prefer-tarray-over-sktdarray
description: When buffering bytes or POD data in dynamic arrays, prefer skia_private::TArray over legacy SkTDArray and use push_back_n instead of append
rubric: design
severity: should_fix
---

**Rule.** Prefer skia_private::TArray<T> (or STArray<N, T>) over legacy
SkTDArray<T> for dynamic arrays, including byte and POD buffers (such as char or
uint8_t). When appending uninitialized space to trivial-type arrays, use
push_back_n(count) instead of SkTDArray::append(); default-initialization via
placement new is a compiler no-op for trivial types that compiles identically to
append() without overhead.

**Why.** Skia is deprecating and consolidating away from legacy SkTDArray in
favor of skia_private::TArray. Developers sometimes reach for SkTDArray under
the mistaken belief that TArray incurs initialization penalties for raw byte
buffers, but push_back_n() on trivial types optimizes down identically to
append() while adhering to modern codebase container conventions.

## Examples

Bad:

```cpp
#include "include/private/base/SkTDArray.h"

SkTDArray<char> fStorage;
// ...
char* dst = fStorage.append(size + padding);
```

Good:

```cpp
#include "include/private/base/SkTArray.h"

skia_private::TArray<char> fStorage;
// ...
char* dst = fStorage.push_back_n(size + padding);
```

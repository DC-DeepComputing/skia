---
name: skia/no-abseil-types-or-suggestions
description: When introducing dependencies or configuring banned C++ feature suggestions, do not use or recommend Abseil (absl::*) types; use Skia internal primitives instead
rubric: design
severity: should_fix
---

**Rule.** Do not use, include, or recommend Abseil (`absl::*`) types in Skia
code or presubmit checks. Skia does not link against Abseil in general builds
(Abseil is only pulled in externally for Dawn). Use Skia's own primitives (such
as `SkMutex` and `SkSharedMutex`) or leave presubmit replacement suggestions
blank.

**Why.** Skia avoids external dependencies like Abseil to keep build overhead
minimal and maintain standalone portability across embedders. Introducing
`absl::` headers, types, or presubmit recommendations causes build breakage or
misleads developers into using unsupported dependencies outside Dawn.

## Examples

Bad:

```cpp
# In PRESUBMIT.py:
(r'std::mutex', 'absl::Mutex'),
(r'std::thread', 'absl::Thread'),

// In C++ source:
#include "absl/synchronization/mutex.h"
absl::Mutex fLock;
```

Good:

```cpp
# In PRESUBMIT.py:
(r'std::mutex', 'SkMutex'),
(r'std::thread', ''),

// In C++ source:
#include "include/private/base/SkMutex.h"
SkMutex fLock;
```

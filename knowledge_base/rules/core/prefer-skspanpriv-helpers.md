---
name: skia/core/prefer-skspanpriv-helpers
description: When comparing or copying SkSpan sequences in core routines, prefer SkSpanPriv helpers like EQ with pointer-identity fast paths, and omit redundant size pre-checks
rubric: safety
severity: should_fix
---

**Rule.** When comparing or copying contiguous spans (SkSpan<T>) within core
routines, use centralized helpers in src/core/SkSpanPriv.h (such as
SkSpanPriv::EQ and SkSpanPriv::Copy) rather than writing manual index-based
loops, implementing ad-hoc helper templates, or open-coding std::equal. Omit
redundant manual size pre-checks before calling comparison helpers that verify
sizes internally. When implementing or extending span equality helpers, verify
matching sizes, handle empty spans, and fast-path pointer identity (a.data() ==
b.data()) before falling back to element-wise std::equal.

**Why.** Centralizing span utilities in SkSpanPriv avoids redundant template
implementations and manual index-loop boilerplate across translation units while
providing bounds verification and O(1) fast paths. Checking pointer equality
(a.data() == b.data()) after verifying matching sizes short-circuits comparisons
when spans share the same underlying buffer, avoiding O(N) element traversal.
Because SkSpanPriv::EQ verifies sizes and empty states internally, manual
caller-side size checks are redundant.

## Examples

Bad:

```cpp
// Bad: ad-hoc helper template without pointer fast-path, or redundant size pre-checks
if (a.fPoints.size() != b.fPoints.size() || a.fVerbs.size() != b.fVerbs.size()) {
    return false;
}
template <typename T> bool spaneq(SkSpan<T> a, SkSpan<T> b) {
    if (a.size() != b.size()) return false;
    return std::equal(a.begin(), a.end(), b.begin());
}
```

Good:

```cpp
#include "src/core/SkSpanPriv.h"

// Good: delegate directly to SkSpanPriv::EQ without manual size checks
bool SkPathData::operator==(const SkPathData& other) const {
    return SkSpanPriv::EQ(this->fPoints, other.fPoints) &&
           SkSpanPriv::EQ(this->fVerbs,  other.fVerbs);
}

// Good: span equality helpers check sizes, empty spans, and pointer identity
template <typename T> static bool EQ(SkSpan<T> a, SkSpan<T> b) {
    if (a.size() != b.size()) return false;
    if (a.empty()) return true;
    return (a.data() == b.data()) || std::equal(a.begin(), a.end(), b.begin());
}
```

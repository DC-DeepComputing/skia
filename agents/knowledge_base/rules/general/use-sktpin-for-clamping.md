---
name: skia/use-sktpin-for-clamping
description: When bounding or clamping numeric values, use SkTPin for ranges and std::min/max for limits instead of custom clamp helpers, if branches, or std::clamp
rubric: readability
severity: should_fix
---

**Rule.** When clamping or pinning numeric values between lower and upper
bounds, use Skia's utility helper SkTPin(val, min, max) rather than defining
ad-hoc clamp functions, open-coding nested std::max/std::min calls, invoking
std::clamp, or using mutable variables with if branches. When bounding a value
to a single upper or lower limit, initialize the variable directly (qualified
with const) using std::min or std::max rather than declaring a mutable variable
and capping it in a subsequent if block. For narrowing conversions of clamped
values, wrap the result in checked helpers like SkToU32 or SkTo<T> instead of
raw static_cast.

**Why.** SkTPin and standard algorithms (std::min, std::max) express bounding
intent concisely, eliminate duplicate ad-hoc clamp templates, remove unnecessary
branching, and allow resulting variables to be declared const. In addition,
SkTPin safely handles NaN by returning the lower bound (unlike std::clamp), and
SkTo<T> verifies in debug builds via assertions that the bounded value safely
fits the destination type.

## Examples

Bad:

```cpp
namespace {
template <typename T>
constexpr const T& clamp(const T& val, const T& min, const T& max) {
    return (val < min) ? min : (val > max ? max : val);
}
}
int64_t alpha = clamp((raw - kZero) / kDiv, kMinU8, kMaxU8);
const uint32_t q = static_cast<uint32_t>(std::max(0, std::min(100, opt.val)));
```

Good:

```cpp
#include "include/private/SkTPin.h"
#include "include/private/base/SkTo.h"

int64_t alpha = SkTPin((raw - kZero) / kDiv, kMinU8, kMaxU8);
const uint32_t q = SkToU32(SkTPin(opt.val, 0, 100));
```

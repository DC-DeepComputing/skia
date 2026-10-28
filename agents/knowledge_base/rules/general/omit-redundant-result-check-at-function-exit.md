---
name: skia/omit-redundant-result-check-at-function-exit
description: When a status variable already holds the success value on fallthrough, return it directly instead of checking result != kSuccess before returning kSuccess
rubric: readability
severity: nit
---

**Rule.** When a status or result variable is already initialized to or
guaranteed to hold the success value on the fallthrough path, return the
variable directly at function exit. Do not write redundant error-checking
branches (such as `if (result != kSuccess) return result; return kSuccess;`)
when returning the status variable achieves the exact same outcome on all paths.

**Why.** Redundant checks against success sentinels immediately before returning
that same sentinel add unnecessary branching and obscure control flow. Returning
the status variable directly simplifies the function tail and handles both
conditional updates and fallthrough cleanly.

## Examples

Bad:

```cpp
SkCodec::Result result = SkCodec::kSuccess;
if (needsMoreFrames) {
    result = frameHolder.appendNewFrame(reader);
    if (result != SkCodec::kSuccess) {
        return result;
    }
}
return SkCodec::kSuccess;
```

Good:

```cpp
SkCodec::Result result = SkCodec::kSuccess;
if (needsMoreFrames) {
    result = frameHolder.appendNewFrame(reader);
}
return result;
```

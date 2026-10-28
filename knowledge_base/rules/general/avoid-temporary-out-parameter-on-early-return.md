---
name: skia/avoid-temporary-out-parameter-on-early-return
description: When passing an out-parameter to a fallible function where failure returns early, pass the destination variable directly rather than declaring a temporary copy
rubric: readability
severity: nit
---

**Rule.** When passing an out-parameter to a fallible function whose failure
causes the caller to immediately return or abort, pass the destination variable
directly. Do not declare an intermediate temporary variable solely to stage the
output before copying it on success, unless a prior fallback value must be
preserved upon failure.

**Why.** When the calling function immediately aborts or returns on failure, the
target variable's previous state does not need to be preserved. Declaring a
temporary variable and performing a redundant copy on success adds visual
clutter and extra operations without providing safety or transactional benefit.

## Examples

Bad:

```cpp
skcms_Matrix3x3 toXYZD50;
if (reader.has_chrm()) {
    skcms_Matrix3x3 tmp;
    if (ToSkColorSpacePrimaries(chrm).toXYZD50(&tmp)) {
        toXYZD50 = tmp;
    } else {
        return nullptr;
    }
}
```

Good:

```cpp
skcms_Matrix3x3 toXYZD50;
if (reader.has_chrm()) {
    if (!ToSkColorSpacePrimaries(chrm).toXYZD50(&toXYZD50)) {
        return nullptr;
    }
}
```

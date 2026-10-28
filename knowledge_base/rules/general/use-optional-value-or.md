---
name: skia/use-optional-value-or
description: When extracting from std::optional with a default fallback, use value_or() instead of ternary or if-else unpacking, and dereference directly when presence is guaranteed
rubric: readability
severity: blocking
---

**Rule.** When extracting an underlying value from a std::optional<T> with a
default fallback, call opt.value_or(fallback) instead of unpacking through a
ternary condition (such as opt.has_value() ? *opt : fallback) or manual if/else
branching. When accessing an optional whose presence has already been verified
(such as after has_value()) or is guaranteed by an invariant, dereference it
directly with operator* or operator->; do not call opt.value(), which performs
redundant checking and throws exceptions if disengaged, and avoid adding
misleading value_or() dummy fallbacks when the value cannot be absent.

**Why.** Calling .value_or() expresses fallback intent directly in a single
concise expression and avoids ternary or if/else unwrapping boilerplate. When an
optional is already verified or guaranteed to hold a value, dereferencing
directly with operator* or operator-> is idiomatic and avoids throwing
std::bad_optional_access (or runtime abort overhead in no-exception builds) and
misleading readers with dead fallback paths.

## Examples

Bad:

```cpp
// Redundant ternary or if/else fallback unwrapping
int width = requestedWidth.has_value() ? *requestedWidth : style.width();
if (auto path = this->getPath(glyphID)) {
    return *path;
} else {
    return SkPath();
}

// Verified presence: calling .value() instead of operator*
UniquePaintParamsID paintID = keyResult.has_value() ? std::get<0>(keyResult.value())
                                                    : fallbackID;

// Guaranteed presence: calling .value() or adding misleading fallback
srcPts[0] = this->getLastPt().value();
srcPts[0] = this->getLastPt().value_or(SkPoint::Make(0, 0));
```

Good:

```cpp
// Express fallback directly with value_or
int width = requestedWidth.value_or(style.width());
return this->getPath(glyphID).value_or(SkPath());

// Verified presence: dereference directly
UniquePaintParamsID paintID = keyResult.has_value() ? std::get<0>(*keyResult)
                                                    : fallbackID;

// Guaranteed presence: dereference directly
srcPts[0] = *this->getLastPt();
```

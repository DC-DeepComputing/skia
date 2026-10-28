---
name: skia/prefer-tarray-push-back-n-over-memcpy
description: When appending an array, span, or repeated value to skia_private::TArray or STArray, use multi-argument push_back_n instead of calling memcpy or memset on single-argument push_back_n
rubric: readability
severity: nit
---

**Rule.** Prefer using skia_private::TArray (or STArray) multi-argument
overloads push_back_n(count, data) and push_back_n(count, val) to copy or fill
elements, rather than calling memcpy or memset on the buffer returned by the
single-argument push_back_n(count). Pass the source data pointer (such as
span.data()) directly to push_back_n.

**Why.** TArray provides built-in overloads push_back_n(int n, const T t[]) and
push_back_n(int n, const T& t) that directly copy arrays or fill elements while
managing internal allocation and element construction. Calling memcpy or memset
manually requires calculating byte counts (e.g. count * sizeof(T)), bypassing
container type safety and risking buffer sizing errors.

## Examples

Bad:

```cpp
// Appending a span/array of elements
memcpy(fVerbs.push_back_n(verbs.size()), verbs.begin(), verbs.size_bytes());

// Sizing with element count * sizeof(T)
memcpy(fPts.push_back_n(count), pts.data(), count * sizeof(SkPoint));

// Filling with a repeated value
memset(fVerbs.push_back_n(count), static_cast<uint8_t>(SkPathVerb::kLine), count);
```

Good:

```cpp
// Appending a span/array of elements
fVerbs.push_back_n(verbs.size(), verbs.data());

// Directly passing pointer and count
fPts.push_back_n(count, pts.data());

// Filling with a repeated value
fVerbs.push_back_n(count, SkPathVerb::kLine);
```

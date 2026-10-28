---
name: skia/avoid-const-ref-for-sink-parameters
description: When transferring ownership of parameters into member variables or sinks, pass by non-const value or rvalue reference rather than const reference or const value
rubric: readability
severity: should_fix
---

**Rule.** Do not declare parameters as const references (const T&) or const
values (const T, such as const sk_sp<T>) when transferring ownership of the
parameter or its fields (e.g., within wrappers like std::optional<T>) into
member variables or local sinks. Take them by non-const value (T) or rvalue
reference (T&&) and std::move them into place so move semantics are invoked
rather than silently falling back to copy construction or atomic ref-count
churn.

**Why.** Invoking std::move() on a const reference or const by-value parameter
produces const T&&, which cannot bind to move constructors or move assignments
and silently falls back to copy construction. Qualifying sink parameters with
const causes redundant atomic ref-count churn for smart pointers (sk_sp<T>) and
prevents moving heap allocations or internal fields.

## Examples

Bad:

```cpp
// Bad: Moving a const reference or const value silently invokes copy construction
SkTypeface_AndroidNDK(const TArray<SkLanguage>& lang)
    : fLang(std::move(lang)) {}

PrecompileColorSpaceXformColorFilter(const sk_sp<SkColorSpace> src)
    : fSrc(std::move(src)) {}

// Bad: Passing const optional reference prevents moving fields into sinks
void SkScalerContext::internalGetPath(
        const std::optional<GeneratedPath>& generatedPath) {
    if (generatedPath) {
        path = generatedPath->path; // Redundant deep copy
    }
}
```

Good:

```cpp
// Good: Taking rvalue reference or non-const value enables move construction
SkTypeface_AndroidNDK(TArray<SkLanguage>&& lang)
    : fLang(std::move(lang)) {}

PrecompileColorSpaceXformColorFilter(sk_sp<SkColorSpace> src)
    : fSrc(std::move(src)) {}

// Good: Taking rvalue reference allows moving internal fields
void SkScalerContext::internalGetPath(
        std::optional<GeneratedPath>&& generatedPath) {
    if (generatedPath) {
        path = std::move(generatedPath->path);
    }
}
```

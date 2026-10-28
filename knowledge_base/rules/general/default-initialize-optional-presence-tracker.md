---
name: skia/default-initialize-optional-presence-tracker
description: When declaring std::optional variables to track presence or conditional values, leave them default-initialized or use std::nullopt instead of assigning dummy values like 0
rubric: correctness
severity: should_fix
---

**Rule.** When declaring a std::optional<T> variable to track presence or
conditionally receive a value, leave it default-initialized or explicitly
initialize it to std::nullopt. Do not initialize it with a dummy or sentinel
value (such as 0, false, or ""), which creates an engaged optional that causes
.has_value() and .value_or() fallback checks to fail.

**Why.** Initializing std::optional<T> with a literal value implicitly
constructs an engaged optional holding that value, causing .has_value() to
evaluate to true before any assignment occurs. This defeats presence checks and
prevents downstream fallback logic from executing when the property is absent.

## Examples

Bad:

```cpp
// Bad: Initializing with 0 constructs an engaged optional holding 0
std::optional<int> requestedWidth = 0;
for (size_t i = 0; i < requestAxisCount; ++i) {
    if (tag == wdth) {
        requestedWidth = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(value);
    }
}
int width = requestedWidth.value_or(style.width()); // Evaluates to 0 even if tag was never found!
```

Good:

```cpp
// Good: Default initialization leaves std::optional disengaged until explicitly set
std::optional<int> requestedWidth; // Or: = std::nullopt;
for (size_t i = 0; i < requestAxisCount; ++i) {
    if (tag == wdth) {
        requestedWidth = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(value);
    }
}
int width = requestedWidth.value_or(style.width()); // Accurately falls back when absent
```

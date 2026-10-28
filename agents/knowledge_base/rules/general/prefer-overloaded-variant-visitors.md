---
name: skia/prefer-overloaded-variant-visitors
description: When visiting variants with std::visit or custom visitors, dispatch using overloaded functions or visitor structs instead of if-constexpr chains
rubric: readability
severity: should_fix
---

**Rule.** When visiting a variant with std::visit or custom variant visitors,
avoid using a monolithic generic lambda containing an if constexpr
(std::is_same...) chain to inspect types. Instead, dispatch by passing an
overloaded function set or an overloaded visitor struct that leverages C++
overload resolution directly for each variant type.

**Why.** Variant visitation is designed to leverage C++ overload resolution for
type-safe static dispatch. Manually branching through an if constexpr
(std::is_same...) chain reconstructs ad-hoc type-tag switching with heavy
template boilerplate and unnecessary cognitive load.

## Examples

Bad:

```cpp
skstd::visit(
        [stream](const auto& arg) {
            using T = typename std::decay<decltype(arg)>::type;
            if constexpr (std::is_same<T, bool>::value) {
                stream->writeText(arg ? "true" : "false");
            } else if constexpr (std::is_same<T, int32_t>::value) {
                stream->writeDecAsText(arg);
            }
        },
        *this);
```

Good:

```cpp
static void emit(bool v, SkWStream* s) { s->writeText(v ? "true" : "false"); }
static void emit(int32_t v, SkWStream* s) { s->writeDecAsText(v); }

void SkPDFEmit(const SkPDFObject& obj, SkWStream* stream) {
    skstd::visit([stream](const auto& v) { emit(v, stream); }, obj);
}
```

---
name: skia/initialize-string-pointer-to-empty-literal
description: When declaring optional or lazily-initialized const char* pointers converted to std::string or std::string_view, initialize them to empty string literals rather than nullptr
rubric: readability
severity: should_fix
---

**Rule.** Default or initialize optional and lazily-loaded const char* string
pointers to an empty string literal ("") rather than nullptr when downstream
consumers convert them to std::string or std::string_view. Do not rely on
nullptr sentinels that require defensive branching or ternary null-checks before
string construction.

**Why.** In C++, constructing std::string or std::string_view from a null
pointer is undefined behavior. Initializing string pointers to "" guarantees
they point to a valid null-terminated string at all times, enabling safe
unconditional conversion (e.g. std::string(ptr)) and eliminating defensive
null-checking boilerplate.

## Examples

Bad:

```cpp
static const char* sdata_shader_source = nullptr;

// Constructing std::string from nullptr is UB, requiring defensive branching
std::string GetShaderSource() {
    return sdata_shader_source ? std::string(sdata_shader_source) : "";
}
```

Good:

```cpp
static const char* sdata_shader_source = "";

// Always points to a valid null-terminated string; safe to convert unconditionally
std::string GetShaderSource() {
    return std::string(sdata_shader_source);
}
```

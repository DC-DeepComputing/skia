---
name: skia/format-string-view-with-precision-in-printf
description: When formatting a std::string_view with printf-style functions, format with %.*s using length and data rather than constructing a temporary std::string
rubric: safety
severity: should_fix
---

**Rule.** When formatting a std::string_view (or length-delimited buffer) with
printf-style formatters (such as SkStringPrintf, SkDebugf, or String::printf),
use the %.*s specifier and pass the length as (int)view.length() (or
(int)view.size()) and pointer as view.data(). Do not construct an intermediate
std::string or SkString merely to obtain a null-terminated C-string via
.c_str(). When formatting non-lvalue expressions, store the std::string_view in
a local variable first to avoid redundant evaluations or dangling temporaries.

**Why.** Converting a std::string_view to a temporary std::string or SkString
forces a dynamic memory allocation and string copy solely to provide a
null-terminated buffer for %s. The %.*s specifier instructs printf to read
directly from the non-null-terminated buffer up to the specified character count
without copying or allocating memory.

## Examples

Bad:

```cpp
// Constructing temporary std::string just to call .c_str() in printf
return String::printf("vec%d<%s>",
                      type.columns(),
                      std::string(to_scalar_type(type.componentType())).c_str());

SkDebugf("Identifier: %s\n", std::string(identifierView).c_str());
```

Good:

```cpp
// Use %.*s with length and data pointers directly without allocation
std::string_view ct = to_scalar_type(type.componentType());
return String::printf("vec%d<%.*s>", type.columns(), (int)ct.length(), ct.data());

SkDebugf("Identifier: %.*s\n", (int)identifierView.size(), identifierView.data());
```

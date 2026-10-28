---
name: skia/api/prefer-string-view-in-options-structs
description: In ephemeral options, descriptor, or configuration structs passed to factory functions, store string fields as std::string_view rather than owning SkString
rubric: safety
severity: should_fix
---

**Rule.** In ephemeral options, descriptor, and configuration structs passed to
factory or initialization functions, store string parameters as std::string_view
(or const char*) rather than owning string types like SkString or std::string.
Leave string allocation and ownership to the long-lived instantiated object.

**Why.** Options and descriptor structs are short-lived and typically passed by
const reference during construction. Using std::string_view avoids unnecessary
heap allocations and string copies when callers pass string literals or views,
deferring memory allocation to the created object.

## Examples

Bad:

```cpp
struct Options {
    bool forceUnoptimized = false;
    SkString fName;
};
```

Good:

```cpp
struct Options {
    bool forceUnoptimized = false;
    std::string_view fName;
};
```

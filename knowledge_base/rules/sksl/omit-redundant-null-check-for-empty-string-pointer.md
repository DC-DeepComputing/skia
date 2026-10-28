---
name: skia/sksl/omit-redundant-null-check-for-empty-string-pointer
description: When converting C-style string pointers guaranteed non-null or initialized to empty literals to std::string, return std::string(ptr) directly without defensive null checks
rubric: readability
severity: nit
---

**Rule.** When converting a const char* pointer to std::string where the pointer
is guaranteed non-null or default-initialized to an empty string literal (""),
construct and return std::string(ptr) directly. Do not retain defensive null
checks (such as if (ptr) return std::string(ptr); else return ""; or ptr ?
std::string(ptr) : "") when null is impossible and an empty string literal
already safely constructs an empty std::string.

**Why.** Constructing std::string from nullptr invokes undefined behavior, but
std::string("") safely constructs an empty string. When a pointer is guaranteed
non-null by initialization, guarding against null creates unreachable dead
branches and visual clutter.

## Examples

Bad:

```cpp
// Redundant null check when pointer is default-initialized to "":
#define G(name)                               \
    case ModuleType::name:                    \
        if (sdata_##name) {                   \
            return std::string(sdata_##name); \
        } else {                              \
            return "";                        \
        }
```

Good:

```cpp
// Construct std::string directly without defensive branching:
#define G(name) case ModuleType::name: return std::string(sdata_##name);
```

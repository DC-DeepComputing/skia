---
name: skia/match-printf-format-specifiers-to-types
description: When formatting integer variables in printf-style functions, format them directly using matching specifiers like %d or %u rather than std::to_string or int casts
rubric: safety
severity: should_fix
---

**Rule.** When formatting numeric variables or expressions with printf-style
functions (such as SkStringPrintf, SkDebugf, or String::printf), use the native
format specifier that matches the variable's type (e.g., %d for signed integers,
%u for uint32_t, %zu for size_t). Do not convert integers to temporary strings
via std::to_string(...).c_str() to format with %s, and do not cast unsigned or
size values to int merely to satisfy %d.

**Why.** Converting integers to temporary strings via
std::to_string(...).c_str() incurs unnecessary intermediate heap allocations and
string conversions when printf format specifiers already format numbers
directly. Similarly, casting unsigned values or container sizes to signed int to
satisfy %d introduces cast clutter and risks incorrect negative numbers or
sign-extension issues when values exceed INT_MAX.

## Examples

Bad:

```cpp
// Converting integers to temporary std::string for %s
String::printf("vec%s<%s>", std::to_string(type.columns()).c_str(), ct.data());

// Casting unsigned integers or container sizes to int to satisfy %d
case Signed: numeric = SkStringPrintf("%d", (int)channelValue); break;
SkDebugf("Buffer size: %d bytes\n", static_cast<int>(buffer.size()));
```

Good:

```cpp
// Formatting integers directly with native %d specifier
String::printf("vec%d<%.*s>", type.columns(), (int)ct.length(), ct.data());

// Matching format specifier directly to variable type without casts
case Signed: numeric = SkStringPrintf("%u", channelValue); break;
SkDebugf("Buffer size: %zu bytes\n", buffer.size());
```

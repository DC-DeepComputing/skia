---
name: skia/do-not-compare-cstring-pointer-to-string-literal
description: When checking raw C-strings for emptiness or content, do not compare pointers against string literals with == or !=; inspect characters or use string helpers
rubric: correctness
severity: blocking
---

**Rule.** Do not compare raw C-string pointers (const char* or char*) against
string literals (such as "") using == or != to test for emptiness or string
contents. Instead, test whether the first character is the null terminator
(str[0] == '\0' or *str == '\0'), use strlen(), or compare via strcmp() /
std::string_view.

**Why.** Comparing a raw C-string pointer against a string literal with == or !=
compares pointer addresses rather than string contents. Because string literal
addresses depend on linker pooling and relocations, address equality yields
unspecified behavior, triggers compiler warnings (-Wstring-compare), and fails
to test string emptiness.

## Examples

Bad:

```cpp
// Bad: compares pointer address against the address of literal ""
SkASSERTF(sdata_sksl_frag == "", "We should only initialize this once");
if (moduleSource != "") {
    process(moduleSource);
}
```

Good:

```cpp
// Good: check first character for null terminator or check pointer validity
SkASSERTF(sdata_sksl_frag[0] == '\0', "We should only initialize this once");
if (moduleSource && moduleSource[0] != '\0') {
    process(moduleSource);
}
```

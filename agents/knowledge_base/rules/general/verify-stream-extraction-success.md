---
name: skia/verify-stream-extraction-success
description: When parsing formatted input or tokens using stream extraction operators, verify extraction success or assert with SkASSERT(!ss.fail()) before using parsed values
rubric: correctness
severity: should_fix
---

**Rule.** When parsing formatted input or tokens using C++ stream extraction
operators (such as `std::istringstream >> ...`), do not assume extraction
succeeds; verify extraction success or assert `!stream.fail()` (such as
`SkASSERT(!ss.fail())`) before using the extracted values. In internal parsing
helpers where format invariants are expected, encapsulate extraction in a helper
that asserts success.

**Why.** Stream extraction operators (`>>`) fail silently by setting
`std::ios_base::failbit` rather than throwing exceptions or halting execution.
Unchecked stream extractions allow uninitialized or stale variables to propagate
downstream, causing silent data corruption, logic errors, or undefined behavior.

## Examples

Bad:

```cpp
if (tokens.size() >= 2 && tokens[0].size() > 1) {
    std::istringstream ss(tokens[0].c_str() + 1);
    ss >> std::hex >> fAsmStart;
    // fAsmStart may remain uninitialized or stale if extraction fails
}
```

Good:

```cpp
size_t getAddr(const char* str) {
    size_t addr = 0;
    std::istringstream ss(str);
    ss >> std::hex >> addr;
    SkASSERT(!ss.fail());
    return addr;
}
```

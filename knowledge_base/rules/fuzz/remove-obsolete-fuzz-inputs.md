---
name: skia/fuzz/remove-obsolete-fuzz-inputs
description: When updating fuzz tests in fuzz/ for API changes, delete obsolete fuzz input reads (fuzz->next) rather than preserving dead reads for corpus compatibility
rubric: readability
severity: should_fix
---

**Rule.** When updating fuzz tests in fuzz/ to reflect API changes, delete
obsolete fuzz input reads (`fuzz->next(...)` or `fuzz->nextRange(...)`) and
their associated unused variables instead of retaining dead reads to preserve
existing fuzz corpus compatibility.

**Why.** Retaining dead fuzz input reads wastes fuzzer byte budget, clutters
test harnesses with unused variables, and misleads maintainers. While altering
the input byte stream can invalidate existing corpus seeds, keeping the fuzzer
harness clean and aligned with the active API outweighs preserving backwards
compatibility with legacy seed corpora.

## Examples

Bad:

```cpp
// In fuzz/FuzzPathop.cpp, retaining an obsolete read after an API parameter is removed:
bool isSame;
fuzz->next(&isSame);
std::ignore = Simplify(path);
```

Good:

```cpp
// Delete the obsolete fuzz->next call and unused variable:
std::ignore = Simplify(path.makeFillType(ft));
```

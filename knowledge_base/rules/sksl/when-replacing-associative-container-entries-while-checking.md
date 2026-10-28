---
name: skia/sksl/when-replacing-associative-container-entries-while-checking
description: When replacing associative container entries while checking prior existence, use std::swap on the slot to install the value and inspect it for early return
rubric: readability
severity: nit
---

**Rule.** When inserting or replacing an entry in an associative container slot
where prior existence must be detected, use std::swap on the slot to install the
new value and inspect the prior entry in a single step. Return early or report
errors immediately based on the swapped-out value instead of introducing mutable
boolean tracking flags.

**Why.** Swapping directly into the container slot combines installation of the
new value and retrieval of the prior value into a single operation. This avoids
maintaining intermediate boolean flags to defer error handling across branches
and preserves flat early-return control flow.

## Examples

Bad:

```cpp
bool foundDuplicate = false;
Symbol*& existing = fSymbols[key];
if (existing != nullptr) {
    foundDuplicate = true;
}
existing = symbol;

if (foundDuplicate) {
    errors->error(pos, "symbol was already defined");
    return false;
}
return true;
```

Good:

```cpp
std::swap(symbol, fSymbols[key]);
if (symbol != nullptr) {
    errors->error(pos, "symbol was already defined");
    return false;
}
return true;
```

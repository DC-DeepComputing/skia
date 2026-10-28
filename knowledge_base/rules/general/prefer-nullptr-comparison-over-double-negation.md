---
name: skia/prefer-nullptr-comparison-over-double-negation
description: When testing pointer validity, use explicit ptr != nullptr for boolean returns and assignments, and direct contextual conversion (if (ptr)) in conditional guards
rubric: readability
severity: should_fix
---

**Rule.** When checking or returning pointer or smart pointer validity in a
boolean context (such as return statements or variable assignments), explicitly
compare against nullptr ('ptr != nullptr') instead of using double negation
('!!ptr'). In conditional guards (such as 'if' or 'while'), evaluate the pointer
or smart pointer directly using contextual bool conversion ('if (ptr)'),
avoiding '!!ptr' or redundant and inconsistent explicit comparisons like 'if
(ptr != nullptr)'.

**Why.** Comparing explicitly against nullptr ('ptr != nullptr') in assignments
and returns clearly communicates pointer validity semantics while eliminating
archaic, unidiomatic double negation ('!!'). In conditional guards, direct
contextual conversion ('if (ptr)') avoids syntactic noise and maintains visual
consistency across predicates within the codebase.

## Examples

Bad:

```cpp
// Double negation to convert pointer to bool
bool isDrawable() const { return !!fDrawable; }

bool isValid() const {
    return !!fBuffer;
}

// Inconsistent or verbose null check in conditional guards
if (!!fTexture) {
    draw(fTexture);
}
if (fTail != nullptr) {
    fTail->fNext = block;
}
```

Good:

```cpp
// Explicit comparison against nullptr in boolean returns
bool isDrawable() const { return fDrawable != nullptr; }

bool isValid() const {
    return fBuffer != nullptr;
}

// Direct contextual conversion in conditional guards
if (fTexture) {
    draw(fTexture);
}
if (fTail) {
    fTail->fNext = block;
}
```

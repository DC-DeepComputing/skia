---
name: skia/check-validity-flag-before-reading-conditional-member
description: When reading or copying conditionally initialized member variables, gate access behind their validity check or else branch instead of unconditionally reading
rubric: safety
severity: should_fix
---

**Rule.** Do not read, copy, or pass conditionally initialized member variables
before verifying their corresponding validity or configuration flag. Gate member
access inside the valid branch or assign fallback local variables conditionally
(such as within an else block or via a ternary expression) rather than
pre-initializing local variables with indeterminate member data.

**Why.** Unconditionally copying or reading a member variable that is only
initialized under specific conditions reads uninitialized or indeterminate
memory when the condition is false. In C++, evaluating uninitialized memory
triggers undefined behavior and MemorySanitizer (MSan) failures, even if the
destination variable is overwritten later in the function.

## Examples

Bad:

```cpp
// Bad: Reading fTF unconditionally reads uninitialized memory when fUseDstTF is true.
skcms_TransferFunction tf = fTF;
if (fUseDstTF) {
    SkAssertResult(dstCS->isNumericalTransferFn(&tf));
}
```

Good:

```cpp
// Good: Declare tf without copying fTF, and read fTF only in the else branch.
skcms_TransferFunction tf;
if (fUseDstTF) {
    SkAssertResult(dstCS->isNumericalTransferFn(&tf));
} else {
    tf = fTF;
}
```

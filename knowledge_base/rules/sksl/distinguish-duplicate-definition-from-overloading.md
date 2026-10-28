---
name: skia/sksl/distinguish-duplicate-definition-from-overloading
description: When diagnosing conflicting function declarations in SkSL, report matching signatures as duplicate definitions rather than claiming intrinsics cannot be overloaded
rubric: correctness
severity: should_fix
---

**Rule.** When rejecting a function declaration in SkSL that matches an existing
intrinsic or built-in function signature, report the diagnostic error as a
duplicate definition rather than stating that the intrinsic cannot be
overloaded. Ensure diagnostics accurately reflect that SkSL permits overloading
intrinsics with distinct parameter signatures, and only identical parameter
profiles constitute illegal redefinitions.

**Why.** SkSL supports user-defined overloads for intrinsic functions with
differing parameter types. Mischaracterizing a redefinition of an identical
signature as an overload error misrepresents language semantics and misleads
shader authors about what function signatures are permitted.

## Examples

Bad:

```cpp
if (other->isIntrinsic()) {
    errors.error(pos, "intrinsic function '" + std::string(name) +
                      "' cannot be overloaded");
    return false;
}
```

Good:

```cpp
if (other->isIntrinsic()) {
    errors.error(pos, "duplicate definition of intrinsic function '" +
                      std::string(name) + "'");
    return false;
}
```

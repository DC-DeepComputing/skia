---
name: skia/avoid-shadowing-functions-with-variable-names
description: When declaring local variables or parameters, avoid naming them after built-in intrinsics or in-scope functions like sign, min, max, or abs
rubric: readability
severity: should_fix
---

**Rule.** Do not name local variables or parameters after built-in math
intrinsics or in-scope functions (such as sign, min, max, or abs). Use a
descriptive, role-specific identifier (such as quadSign, minVal, or xLimit) to
prevent shadowing functions in the local lexical scope.

**Why.** Naming a variable after a built-in intrinsic or function shadows that
function name within the lexical scope, preventing direct calls to the function
and causing confusing expressions like 'sign = sign(...)'. Using a distinct,
semantically descriptive identifier avoids symbol collisions and preserves
readability.

## Examples

Bad:

```cpp
float sign = sign(1 - dRadius);
t = invA * (-b + sign * sqrt(d));
```

Good:

```cpp
float quadSign = sign(1 - dRadius);
t = invA * (-b + quadSign * sqrt(d));
```

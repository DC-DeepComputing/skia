---
name: skia/prefer-sktobool-for-bitwise-flag-checks
description: When explicitly converting values or bitmask flags to bool, use SkToBool instead of static_cast or comparisons to zero, but omit SkToBool in condition statements
rubric: readability
severity: should_fix
---

**Rule.** Use the SkToBool(...) helper instead of static_cast<bool>(...) or
explicit comparisons to zero/kNone (e.g., '!= 0', '!= kNone') when explicitly
assigning or returning boolean values from pointers or bitwise flag expressions.
Conversely, omit SkToBool(...) and explicit comparisons inside contextual
boolean conditions (such as 'if', 'while', or ternary expressions), allowing
integral values, pointers, and types with explicit operator bool() (such as
SkEnumBitMask) to convert directly.

**Why.** SkToBool(...) is Skia's canonical helper when an explicit bool type is
required for assignment or return, replacing verbose static_cast<bool> or
comparisons to 0/kNone. Inside conditional statements, C++ already contextually
converts expressions to bool, making SkToBool(...) redundant clutter.

## Examples

Bad:

```cpp
// Explicit bool assignment or return
bool isValid() const { return static_cast<bool>(fBuffer); }
bool isStencil = (renderer->depthStencilFlags() & DepthStencilFlags::kStencil) !=
                 DepthStencilFlags::kNone;

// Redundant SkToBool or comparison in condition expression
if (SkToBool(fFlags & Flags::kAppendVertices)) { ... }
if ((flags & Mask::kEnabled) != 0) { ... }
```

Good:

```cpp
// Explicit bool assignment or return
bool isValid() const { return SkToBool(fBuffer); }
bool isStencil = SkToBool(renderer->depthStencilFlags() & DepthStencilFlags::kStencil);

// Direct contextual conversion in condition expression
if (fFlags & Flags::kAppendVertices) { ... }
if (flags & Mask::kEnabled) { ... }
```

---
name: skia/omit-return-after-skunreachable
description: When terminating unreachable control paths or switch cases, use SkUNREACHABLE or SK_ABORT instead of SkASSERT(0), and omit dead returns or trailing SkUNREACHABLE
rubric: readability
severity: should_fix
---

**Rule.** Terminate unreachable control-flow paths (such as impossible switch
defaults or dead fallthroughs) with `SkUNREACHABLE;`, `SK_ABORT(...)`, or
`SkDEBUGFAIL(...)` rather than `SkASSERT(false)` or `SkASSERT(0)`. Because
`SkUNREACHABLE` and `SK_ABORT` are `[[noreturn]]`, do not place any `return`,
`break;`, or executable code after them. Similarly, when all cases of a `switch`
return or abort (including via a returning `default:`), do not add a dead
`SkUNREACHABLE;` or fallback return after the switch block; if non-matching
cases return a fallback value, omit `default:` and place the return after the
switch.

**Why.** SkASSERT(0) and SkASSERT(false) compile to no-ops in release builds,
allowing execution to fall through into unhandled code or requiring dead dummy
return statements. In contrast, SkUNREACHABLE and SK_ABORT terminate
unconditionally via [[noreturn]], satisfying non-void return requirements.
Placing trailing returns after SkUNREACHABLE, or placing SkUNREACHABLE after
switches whose branches already return or abort, creates dead code and can
suppress compiler exhaustiveness warnings.

## Examples

Bad:

```cpp
// Bad: Using SkASSERT(false) and dummy return for unreachable paths
switch (dir) {
    case SkPathFirstDirection::kCW:      return SkPathConvexity::kConvex_CW;
    case SkPathFirstDirection::kCCW:     return SkPathConvexity::kConvex_CCW;
    case SkPathFirstDirection::kUnknown: return SkPathConvexity::kConvex_Degenerate;
}
SkASSERT(false);
return SkPathConvexity::kConvex_Degenerate;

// Bad: Combining a returning default case with dead SkUNREACHABLE after switch
switch (imageTiling) {
    case VK_IMAGE_TILING_OPTIMAL: return fIsEfficientWithHostImageCopy;
    case VK_IMAGE_TILING_LINEAR:  return true;
    default:                      return false;
}
SkUNREACHABLE;

// Bad: Placing a dead return statement after SkUNREACHABLE
SkUNREACHABLE;
return SkPathConvexity::kConvex_Degenerate;
```

Good:

```cpp
// Good: Terminate unreachable paths with SkUNREACHABLE and omit trailing return
switch (dir) {
    case SkPathFirstDirection::kCW:      return SkPathConvexity::kConvex_CW;
    case SkPathFirstDirection::kCCW:     return SkPathConvexity::kConvex_CCW;
    case SkPathFirstDirection::kUnknown: return SkPathConvexity::kConvex_Degenerate;
}
SkUNREACHABLE;

// Good: Return fallback value after switch instead of default case and SkUNREACHABLE
switch (imageTiling) {
    case VK_IMAGE_TILING_OPTIMAL: return fIsEfficientWithHostImageCopy;
    case VK_IMAGE_TILING_LINEAR:  return true;
}
return false;

// Good: Use SkUNREACHABLE in impossible default without trailing break or return
default:
    SkUNREACHABLE;
```

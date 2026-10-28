---
name: skia/omit-redundant-conditions-in-known-branches
description: When branching with guards, if-else ladders, or ternaries, eliminate redundant conditions, duplicate pre-dispatch filters, and checks already guaranteed by enclosing flow
rubric: readability
severity: should_fix
---

**Rule.** Do not retain duplicate or redundant conditional checks, nested
guards, null checks, or ternary operators whose condition is already determined
by enclosing control flow or preceding early returns (e.g., checking if (ptr)
after an early-return if (!ptr) guard, or evaluating cond ? a : b inside an else
branch of if (cond)). When moving or consolidating conditional logic into an
enclosing guard block, remove the original trailing check. Similarly, do not
write a preliminary guard check filtering unsupported cases immediately before
an if / else if dispatch chain; handle unsupported cases in a trailing else
branch (e.g., else { continue; } or else { return; }) instead of duplicating
condition checks. In exhaustive branches, do not use a redundant condition in an
else if branch when preceding checks and domain constraints make the alternative
branch exhaustive; use a plain else instead.

**Why.** Evaluating a condition whose truth value is already fixed by an
enclosing block, an else branch, or preceding early exits creates dead code and
unnecessary branch logic. Similarly, filtering unsupported cases with a
preliminary guard before an if / else if chain duplicates condition tests and
risks maintenance divergence when supported variants evolve. In exhaustive
branches, a redundant else if check misleads readers into expecting an unhandled
fallback case and risks uninitialized variable warnings or shader compilation
hazards.

## Examples

Bad:

```cpp
// Redundant pre-filtering guard before if / else if dispatch:
if (!arg.is<VariableReference>() && !arg.is<FieldAccess>()) {
    continue;
}
if (arg.is<VariableReference>()) {
    var = arg.as<VariableReference>().variable();
} else if (arg.is<FieldAccess>()) {
    var = arg.as<FieldAccess>().base()->as<VariableReference>().variable();
}

// Redundant ternary in known else branch:
if (fSkipColorXform) {
    AddPrimitiveColor(keyContext, keyBuilder, gatherer, keyContext.dstColorInfo().colorSpace());
} else {
    const SkColorSpace* primitiveCS =
            fSkipColorXform ? keyContext.dstColorInfo().colorSpace()
                            : sk_srgb_singleton();
    AddPrimitiveColor(keyContext, keyBuilder, gatherer, primitiveCS);
}

// Redundant else if on exhaustive sign check:
if (radiiAndFlags.z > 0) {
    clipPlane = fragClipPlane0;
} else if (radiiAndFlags.z < 0) {
    clipPlane = fragClipPlane1;
}

// Redundant null check following early return:
if (!content) {
    return DrawResult::kSkip;
}
if (content) {
    input = content->makeTemporaryImage();
}
```

Good:

```cpp
// Consolidate unsupported bailout into trailing else:
if (arg.is<VariableReference>()) {
    var = arg.as<VariableReference>().variable();
} else if (arg.is<FieldAccess>()) {
    var = arg.as<FieldAccess>().base()->as<VariableReference>().variable();
} else {
    continue;
}

// Simplified branch value:
if (fSkipColorXform) {
    AddPrimitiveColor(keyContext, keyBuilder, gatherer, keyContext.dstColorInfo().colorSpace());
} else {
    const SkColorSpace* primitiveCS = sk_srgb_singleton();
    AddPrimitiveColor(keyContext, keyBuilder, gatherer, primitiveCS);
}

// Plain else for exhaustive alternative case:
if (radiiAndFlags.z > 0) {
    clipPlane = fragClipPlane0;
} else {
    clipPlane = fragClipPlane1;
}

// Unconditional access following early return:
if (!content) {
    return DrawResult::kSkip;
}
input = content->makeTemporaryImage();
```

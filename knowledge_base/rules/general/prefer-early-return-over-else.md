---
name: skia/prefer-early-return-over-else
description: When handling errors, trivial conditions, or early exits, return early using guard clauses and omit redundant else blocks to keep subsequent code unnested
rubric: readability
severity: nit
---

**Rule.** Prefer returning early from a conditional branch once its processing
is complete rather than wrapping subsequent fallback or alternative logic in an
else block. Similarly, use guard clauses with early returns on failure or for
trivial, empty, or no-op conditions rather than wrapping the remainder of the
function or primary execution path in an if block (e.g., use if (!success)
return false; or if (type == kNone) return {}; instead of wrapping the function
body in if (success) or if (type != kNone)). When an if block already terminates
with an early return, do not write a redundant else or else if branch; unnest
the subsequent conditions or statements into the enclosing scope.

**Why.** Returning early eliminates unnecessary nesting depth and indentation by
keeping primary execution, alternative branches, and subsequent checks un-nested
at the outer scope. Guarding against failure or trivial no-op cases upfront
makes bailouts explicit, while omitting redundant else branches after an early
return keeps control flow flat and avoids misleading readers into expecting
coupled branch alternatives.

## Examples

Bad:

```cpp
// Guard clause: wrapping primary execution path in trivial check
SkTLazy<SkPaint> commonPaint(const SkSVGPaint& paint, float opacity) const {
    SkTLazy<SkPaint> p;
    if (paint.type() != SkSVGPaint::Type::kNone) {
        p.init();
        // ... bulk of function body ...
    }
    return p;
}

// Branch completion: unnecessary else block
if (mask & kScale_Mask) {
    ComputeScale(dst, src);
} else {
    ComputeTranslate(dst, src);
}
return true;

// Redundant else/else-if following early return
if (paramIsSpecialized && !fUseTextureSamplerPairs) {
    return NA;
} else if (arg.is<VariableReference>()) {
    EmitVariable(arg);
}

// Failure check: wrapping success path
bool success = skcms_ApproximateCurve(&curve, &skcms_approx, &max_error);
if (success) {
    memcpy(&out_approx, &skcms_approx, sizeof(skcms_TransferFunction));
    out_max_error = max_error;
}
return success;
```

Good:

```cpp
// Guard clause: return early for trivial condition, keep primary path un-nested
SkTLazy<SkPaint> commonPaint(const SkSVGPaint& paint, float opacity) const {
    if (paint.type() == SkSVGPaint::Type::kNone) {
        return {};
    }
    SkTLazy<SkPaint> p;
    p.init();
    // ... bulk of function body ...
    return p;
}

// Branch completion: return early, keep fallback un-nested
if (mask & kScale_Mask) {
    ComputeScale(dst, src);
    return true;
}
ComputeTranslate(dst, src);
return true;

// Unnested subsequent check after early return
if (paramIsSpecialized && !fUseTextureSamplerPairs) {
    return NA;
}
if (arg.is<VariableReference>()) {
    EmitVariable(arg);
}

// Failure check: bail out early, keep success path un-nested
bool success = skcms_ApproximateCurve(&curve, &skcms_approx, &max_error);
if (!success) {
    return false;
}
memcpy(&out_approx, &skcms_approx, sizeof(skcms_TransferFunction));
out_max_error = max_error;
return true;
```

---
name: skia/assert-function-preconditions-at-entry
description: When asserting function preconditions or caller invariants with SkASSERT or DCHECK, place assertions unconditionally at function entry rather than inside conditional branches
rubric: correctness
severity: should_fix
---

**Rule.** Place function and method precondition assertions (such as SkASSERT or
DCHECK) unconditionally at function entry before conditional branches or early
returns, ensuring caller invariants and argument constraints are validated
across all execution paths. Do not nest precondition assertions inside
conditional branches or fallback paths where bypassing the branch allows invalid
caller state to pass undetected.

**Why.** Placing a function precondition assertion inside an inner conditional
branch or after an early return allows invalid arguments and broken caller
contracts to execute undetected whenever the condition evaluates to false or the
branch is skipped. Validating preconditions unconditionally at function entry
ensures contract violations fail fast across all code paths in debug builds and
test suites.

## Examples

Bad:

```cpp
std::optional<AutoLayerForImageFilter> SkCanvas::attemptBlurredRRectDraw(
        const SkRRect& rrect, const SkPaint& paint, SkEnumBitMask<PredrawFlags> flags) {
    if (const SkBlurMaskFilterImpl* filter = can_attempt_blurred_rrect_draw(paint)) {
        // Bad: Assertion is bypassed whenever can_attempt_blurred_rrect_draw returns false!
        SkASSERT(!(flags & PredrawFlags::kSkipMaskFilterAutoLayer));
        ...
    }
    return std::nullopt;
}
```

Good:

```cpp
std::optional<AutoLayerForImageFilter> SkCanvas::attemptBlurredRRectDraw(
        const SkRRect& rrect, const SkPaint& paint, SkEnumBitMask<PredrawFlags> flags) {
    // Good: Function precondition is validated unconditionally at function entry
    SkASSERT(!(flags & PredrawFlags::kSkipMaskFilterAutoLayer));

    if (const SkBlurMaskFilterImpl* filter = can_attempt_blurred_rrect_draw(paint)) {
        ...
    }
    return std::nullopt;
}
```

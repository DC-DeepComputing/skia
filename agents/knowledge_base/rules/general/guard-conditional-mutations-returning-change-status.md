---
name: skia/guard-conditional-mutations-returning-change-status
description: When conditionally mutating an object and returning whether a change occurred, guard the mutation with an inequality check rather than mutating unconditionally
rubric: readability
severity: nit
---

**Rule.** When a function conditionally mutates an object, member, or
out-parameter and returns a boolean reporting whether state changed, guard the
mutation with an explicit difference check (`if (new_val != old_val) { ...;
return true; } return false;`). Do not unconditionally overwrite or mutate the
target before returning the comparison result.

**Why.** Unconditionally mutating a target before checking whether values differ
performs redundant writes when the value is already identical and can trigger
unnecessary dirty flags, cache invalidations, or observer notifications.
Guarding the mutation with an explicit check clarifies the conditional mutation
intent at a glance and avoids unnecessary state modifications.

## Examples

Bad:

```cpp
bool RenderNode::RenderContext::modulatePaint(SkPaint* paint) const {
    const auto initial_alpha = paint->getAlpha(),
                       alpha = SkToU8(sk_float_round2int(initial_alpha * fOpacity));

    paint->setAlpha(alpha);

    return alpha != initial_alpha;
}
```

Good:

```cpp
bool RenderNode::RenderContext::modulatePaint(SkPaint* paint) const {
    const auto initial_alpha = paint->getAlpha(),
                       alpha = SkToU8(sk_float_round2int(initial_alpha * fOpacity));

    if (alpha != initial_alpha) {
        paint->setAlpha(alpha);
        return true;
    }

    return false;
}
```

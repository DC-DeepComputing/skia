---
name: skia/represent-loop-budgets-as-countdowns
description: When implementing loops with an iteration budget or penalty, do not name incrementing counters limit; use countdown budgets decremented toward zero
rubric: readability
severity: should_fix
---

**Rule.** Do not name an increasing loop counter limit or budget; reserve limit
for the maximum upper bound and budget for the remaining allowance. When
reducing or penalizing the remaining iteration allowance in a bounded loop,
represent the allowance as a countdown budget (initialized to the upper limit
and decremented toward zero) rather than artificially incrementing a progress
counter toward the bound.

**Why.** Calling an incrementing progress counter limit is misleading because
limit conventionally denotes a fixed boundary. Furthermore, penalizing a budget
by incrementing a counter toward a limit inverts natural intuition (adding to a
variable suggests expanding capacity rather than consuming it), whereas
countdown budgets make penalties straightforward subtractions and keep the upper
limit constant.

## Examples

Bad:

```cpp
uint32_t limit = 0;
// Penalize search allowance by incrementing counter toward bound
if (shouldPenalize) {
    limit += kMaxSearchLimit >> 1;
}
for (; limit < kMaxSearchLimit && current != stop; ++limit) {
    if (process(current)) {
        break;
    }
    current = current->next();
}
```

Good:

```cpp
int32_t budget = kMaxSearchLimit;
// Penalize remaining allowance directly by subtraction
if (shouldPenalize) {
    budget -= kMaxSearchLimit >> 1;
}
for (; budget > 0 && current != stop; --budget) {
    if (process(current)) {
        break;
    }
    current = current->next();
}
```

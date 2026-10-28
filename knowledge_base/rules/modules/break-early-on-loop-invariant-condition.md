---
name: skia/modules/break-early-on-loop-invariant-condition
description: When executing an unconditional initial pass followed by conditional iterations on an invariant flag, break early in a while-true loop rather than nesting inside if-else
rubric: readability
severity: nit
---

**Rule.** In loops that execute an initial pass unconditionally and subsequent
iterations conditionally based on an invariant flag or parameter, break early
(`if (!flag) break;`) inside a `while (true)` or `do ... while (true)` loop. Do
not nest the loop body in `if (flag) { ... } else { break; }` or repeat the
invariant condition in `while (flag)`.

**Why.** Using an invariant variable as a while loop condition misleadingly
suggests that the variable is modified inside the loop to control termination.
Breaking early on the invariant flag flattens control flow, avoids redundant
indentation, and clarifies that loop termination is governed by internal
convergence checks.

## Examples

Bad:

```cpp
do {
    // Unconditional initial pass / measurement...
    if (limitToGraphemes) {
        // Conditionally adjust for graphemes...
        if (startIndex == textRange.start && endIndex == textRange.end) {
            break;
        }
        textRange.start = startIndex;
        textRange.end = endIndex;
    } else {
        break;
    }
} while (limitToGraphemes);
```

Good:

```cpp
while (true) {
    // Unconditional initial pass / measurement...
    if (!limitToGraphemes) {
        break;
    }
    // Conditionally adjust for graphemes...
    if (startIndex == textRange.start && endIndex == textRange.end) {
        break;
    }
    textRange.start = startIndex;
    textRange.end = endIndex;
}
```

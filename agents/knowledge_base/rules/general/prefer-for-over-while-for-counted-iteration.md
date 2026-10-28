---
name: skia/prefer-for-over-while-for-counted-iteration
description: When iterating with an index or counter incremented once per iteration, prefer a for loop over a while loop
rubric: readability
severity: nit
---

**Rule.** Prefer a for loop over a while loop when iterating with an index,
counter, or induction variable that advances on each iteration. Consolidate
variable initialization, loop condition, and step increment in the for loop
header rather than declaring the index before a while loop with a trailing
increment.

**Why.** Consolidating initialization, termination condition, and increment in a
for loop header scopes the counter to the loop body, communicates iteration
intent clearly, and avoids skipped increments or infinite loops if continue
statements or multiple branches are introduced.

## Examples

Bad:

```cpp
int i = 0;
int j = 1;
while (j <= fColorCount) {
    if (j == fColorCount || fPositions[i] != fPositions[j]) {
        // ...
        i = j;
    }
    j++;
}
```

Good:

```cpp
int i = 0;
for (int j = 1; j <= fColorCount; ++j) {
    if (j == fColorCount || fPositions[i] != fPositions[j]) {
        // ...
        i = j;
    }
}
```

---
name: skia/pathops/prefer-break-to-trailing-return-in-infinite-loops
description: When resolving unreachable code warnings after infinite or do-while(true) loops in non-void functions, break from the loop rather than removing the trailing return
rubric: readability
severity: should_fix
---

**Rule.** In non-void functions containing an infinite loop (such as while
(true) or do ... while (true)), prefer exiting the loop with break and returning
at the function tail rather than returning directly from within the loop and
leaving the function without a trailing return statement. When resolving
unreachable-code compiler warnings after such loops, do not simply delete the
trailing return if the loop can cleanly break to it.

**Why.** Terminating a non-void function immediately after an infinite loop
without an explicit return statement at function scope obscures exit paths from
readers. Breaking out of the loop and returning at the function tail satisfies
unreachable-code diagnostics while making the primary return path obvious.

## Examples

Bad:

```cpp
bool SkOpAngle::insert(SkOpAngle* angle) {
    do {
        if (angle->after(last) ^ (angle->tangentsAmbiguous() & flipAmbiguity)) {
            last->fNext = angle;
            angle->fNext = next;
            return true;
        }
        ...
    } while (true);
}
```

Good:

```cpp
bool SkOpAngle::insert(SkOpAngle* angle) {
    do {
        if (angle->after(last) ^ (angle->tangentsAmbiguous() & flipAmbiguity)) {
            last->fNext = angle;
            angle->fNext = next;
            break;
        }
        ...
    } while (true);
    return true;
}
```

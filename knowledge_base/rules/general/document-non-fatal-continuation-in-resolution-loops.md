---
name: skia/document-non-fatal-continuation-in-resolution-loops
description: When skipping elements with continue upon lookup, fallback, or resolution failures in a loop, document why the failure is non-fatal and intentionally non-aborting
rubric: readability
severity: nit
---

**Rule.** When handling lookup, fallback, or resolution failures inside a loop
where a missing element or failed match is non-fatal, document in an inline
comment why iteration continues rather than aborting. Clarify that skipping the
item is expected behavior and that subsequent elements must still be resolved.

**Why.** In sequential resolution and fallback loops (such as font matching
during text shaping), a lookup failure can easily be mistaken by maintainers for
an unrecoverable error that should abort the entire operation. An inline comment
prevents regressions where non-fatal skips are mistakenly converted into
premature early exits or error returns.

## Examples

Bad:

```cpp
if (typeface == nullptr) {
    continue;
}
```

Good:

```cpp
if (typeface == nullptr) {
    // There is no fallback font for this character, so move on to the next character.
    continue;
}
```

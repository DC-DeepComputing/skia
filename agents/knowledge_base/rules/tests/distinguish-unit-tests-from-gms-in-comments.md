---
name: skia/tests/distinguish-unit-tests-from-gms-in-comments
description: When commenting on draw sequences or test logic under tests/, refer to draw calls or code under test rather than referencing a GM
rubric: readability
severity: nit
---

**Rule.** When documenting draw sequences, compositing order, or test behavior
in unit tests (`tests/**`), accurately refer to the draw calls or code under
test rather than referencing a 'GM' unless explicitly citing a named Golden
Master rendering test (`gm/`).

**Why.** Skia separates programmatic unit tests (under `tests/`) from Golden
Master rendering tests (under `gm/`). Referring to local test draw operations as
'the GM' when no GM is involved confuses reviewers and maintainers about whether
an existing visual test is being referenced or replicated.

## Examples

Bad:

```cpp
/*
    These tests replicate the compositing behavior in blink's canvas2d. Due to the use of save
    layers for compositing, the final order of draw tasks on the root task list does not match the
    ordering in the GM. At a high level the layer tests look like this:
*/
DEF_GRAPHITE_TEST(NotifyInUseTest, reporter) {
```

Good:

```cpp
/*
    These tests replicate the compositing behavior in blink's canvas2d. Due to the use of save
    layers, the final order of draw tasks on the root task list does not match the order of draw
    calls in the code. At a high level the flow of draws and flushes looks like this:
*/
DEF_GRAPHITE_TEST(NotifyInUseTest, reporter) {
```

---
name: skia/site/document-skia-roll-test-suppressions-section
description: When documenting Blink layout test suppressions in Chromium's TestExpectations for Skia rolls, instruct developers to use the dedicated '# --- Skia roll test suppressions' section
rubric: readability
severity: should_fix
---

**Rule.** When documenting or updating instructions for temporary Blink layout
test suppressions in Chromium's TestExpectations ahead of a Skia roll, instruct
developers to place entries in the dedicated '# --- Skia roll test suppressions'
section.

**Why.** Chromium's third_party/blink/web_tests/TestExpectations file reserves a
dedicated '# --- Skia roll test suppressions' block for roll-related failures.
Isolating Skia roll suppressions in this section minimizes merge conflicts with
concurrent Chromium changes and facilitates cleanup after the roll lands.

## Examples

Bad:

```cpp
Ahead of the Skia auto roll, manually push a change to Blink's TestExpectations file flagging failing tests:
```

Good:

```cpp
Ahead of the Skia auto roll, manually push a change to Blink's TestExpectations file flagging failing tests:
```

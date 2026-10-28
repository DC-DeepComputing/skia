---
name: skia/verify-precondition-comments-match-assertions
description: When writing doc comments specifying caller preconditions or dispatch requirements, verify described conditions match actual implementation semantics and assertions
rubric: readability
severity: should_fix
---

**Rule.** When documenting caller preconditions, requirements, or dispatch
conditions in doc comments, verify that the condition and boolean polarity match
the actual implementation and assertions (such as SkASSERT). Do not state the
inverse polarity or describe calling conditions that contradict the method's
runtime checks.

**Why.** Inverted boolean polarity or contradictory preconditions in comments
mislead callers and future maintainers about when a method is safe to invoke.
Inaccurate doc comments can lead callers to pass invalid state or trigger debug
assertion failures at runtime.

## Examples

Bad:

```cpp
// The same as incrementalDecode but uses a color transformation. Should only
// be used if this->canReadRows() is true.
Result incrementalDecodeColorXForm(DecodingState& decodingState, int* rowsDecoded);
```

Good:

```cpp
// The same as incrementalDecode but uses a color transformation. Should only
// be used if this->canReadRows() is false.
Result incrementalDecodeColorXForm(DecodingState& decodingState, int* rowsDecoded);
```

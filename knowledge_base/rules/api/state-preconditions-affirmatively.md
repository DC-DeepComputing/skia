---
name: skia/api/state-preconditions-affirmatively
description: When documenting API preconditions, options, or ordering requirements in public headers, state conditions affirmatively rather than using ambiguous compound negations
rubric: readability
severity: nit
---

**Rule.** In API doc comments, option descriptions, and interface contracts,
state preconditions and ordering requirements affirmatively rather than using
ambiguous compound negations (such as 'not A or B'). Frame the condition around
what clients must do or what state must hold (e.g., 'replayed in the order they
are recorded' rather than 'not replayed or played out of order').

**Why.** Compound negative conditions like 'not A or B' are grammatically
ambiguous because readers cannot determine whether the negation binds to the
first clause ('(!A) || B') or distributes over both ('!(A || B)'). Stating
requirements and assumptions affirmatively eliminates ambiguity and makes API
contracts immediately clear to callers.

## Examples

Bad:

```cpp
/**
 * If Recordings are not replayed or played out of order, then Graphite
 * can make certain assumptions that allow for better performance.
 */
bool fRequireOrderedRecordings = false;
```

Good:

```cpp
/**
 * If Recordings are replayed in the order they are recorded, then Graphite
 * can make certain assumptions that allow for better performance.
 */
bool fRequireOrderedRecordings = false;
```

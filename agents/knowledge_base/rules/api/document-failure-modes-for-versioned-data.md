---
name: skia/api/document-failure-modes-for-versioned-data
description: When documenting versioned or serialized data in public API comments, state concrete failure modes on version mismatch instead of client regeneration obligations
rubric: readability
severity: should_fix
---

**Rule.** In public API comments for versioned or serialized data, describe the
concrete failure mode when data is incompatible (such as deserialization or
recreation returning null or failing) rather than phrasing versioning as a
client obligation to detect and regenerate stale data. Clients cannot inspect
internal serialization formats ahead of time to detect obsolescence.

**Why.** Clients have no mechanism to parse internal serialization formats or
determine whether cached data is outdated before passing it back into Skia.
Framing versioning as a client duty to detect and regenerate stale data
misplaces responsibility, whereas documenting that deserialization or recreation
can fail on version mismatch accurately reflects API semantics and error
handling.

## Examples

Bad:

```cpp
// When provided, the SkData contains all the information Graphite requires to recreate
// the Pipeline at a later date. The SkData is versioned however, so must be regenerated and
// re-serialized when it becomes out of date.
```

Good:

```cpp
// When provided, the SkData contains all the information Graphite requires to recreate
// the Pipeline at a later date, but it is versioned so recreation can fail if it's
// incompatible with a newer version of Skia.
```

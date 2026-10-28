---
name: skia/sync-virtual-subclass-contract-comments
description: When refactoring base class virtual methods or subclass hooks, update doc comments to reflect changes in caller preconditions and override responsibilities
rubric: readability
severity: should_fix
---

**Rule.** When modifying base class virtual methods or subclass hooks (such as
NVI on* methods), update accompanying doc comments whenever caller
preconditions, default fallback behavior, or override responsibilities change.
Explicitly document what helpers, resource resets, or preconditions derived
classes must invoke or satisfy, rather than leaving obsolete comments that
misstate subclass obligations.

**Why.** When refactoring shifts responsibilities—such as delegating stream
rewinding or state setup to derived overrides or optional helpers rather than
guaranteeing preconditions before dispatch—stale doc comments mislead subclass
implementers into omitting required helper calls or relying on obsolete base
guarantees.

## Examples

Bad:

```cpp
/**
 *  Called by rewindIfNeeded, if the stream needed to be rewound.
 *
 *  Subclasses should do any set up needed after a rewind.
 */
virtual bool onRewind() {
    if (!this->rewindStream()) {
        return false;
    }
    return true;
}
```

Good:

```cpp
/**
 *  Called by rewindIfNeeded, if the stream needed to be rewound.
 *
 *  Subclasses should call rewindStream() if they own one, and then
 *  do any set up needed after.
 */
virtual bool onRewind() {
    if (!this->rewindStream()) {
        return false;
    }
    return true;
}
```

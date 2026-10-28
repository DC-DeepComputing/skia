---
name: skia/gpu/assert-linked-list-traversal-postconditions
description: When appending to or mutating linked lists after traversal loops, assert non-null pointer and null-terminator postconditions with SkASSERT before linking new nodes
rubric: readability
severity: nit
---

**Rule.** When traversing a linked list to append or insert an entry, assert the
loop's postconditions using SkASSERT (such as verifying the target pointer is
non-null and that its next pointer is null) before dereferencing or updating
link pointers. Explicitly asserting traversal termination invariants documents
assumptions and guards against regressions if the traversal logic is refactored.

**Why.** In manual pointer traversals, even when loop control flow currently
guarantees that a pointer is non-null and reaches the tail, asserting these
invariants safeguards against future refactorings that might introduce early
breaks or modify loop termination. It also serves as clear inline documentation
of the list structure invariants at the mutation site.

## Examples

Bad:

```cpp
if (entryList) {
    MaskHashEntry* currEntry = entryList;
    while (currEntry->fNext) {
        currEntry = currEntry->fNext;
    }
    // Missing assertion of traversal postconditions before updating pointer
    currEntry->fNext = new MaskHashEntry{keyBounds, locator, nullptr};
}
```

Good:

```cpp
if (entryList) {
    MaskHashEntry* currEntry = entryList;
    while (currEntry->fNext) {
        currEntry = currEntry->fNext;
    }
    SkASSERT(currEntry);
    SkASSERT(currEntry->fNext == nullptr); // Should be at the end of the list
    currEntry->fNext = new MaskHashEntry{keyBounds, locator, nullptr};
}
```

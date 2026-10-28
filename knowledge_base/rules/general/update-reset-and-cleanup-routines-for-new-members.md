---
name: skia/update-reset-and-cleanup-routines-for-new-members
description: When adding cached members or state containers to classes with reset routines, update all reset, rewind, and cleanup methods to clear them
rubric: correctness
severity: should_fix
---

**Rule.** When adding a new cached container, mapping, or state member to a
class that provides reset, rewind, or post-build cleanup routines (such as
reset(), Reset(), rewind(), or Build()), ensure all corresponding cleanup
methods are updated to clear or reinitialize the new member. Do not update only
a subset of lifecycle teardown paths.

**Why.** Classes with reset or rewind methods are designed to be reused across
operations or build cycles. Failing to clear auxiliary caches or state members
during reset or lifecycle completion leaves stale data behind when instances are
reused, causing stale indexing, desynchronized caches, memory leaks, or
corrupted output.

## Examples

Bad:

```cpp
// Added member fUTF16IndexForUTF8Index, but omitted it from Reset():
void ParagraphBuilderImpl::Reset() {
    fPlaceholders.clear();
    fUTF8IndexForUTF16Index.clear();
    fWordsUtf16.clear();
}
```

Good:

```cpp
// Ensure all auxiliary caches and state members are cleared:
void ParagraphBuilderImpl::Reset() {
    fPlaceholders.clear();
    fUTF8IndexForUTF16Index.clear();
    fUTF16IndexForUTF8Index.clear();
    fWordsUtf16.clear();
}
```

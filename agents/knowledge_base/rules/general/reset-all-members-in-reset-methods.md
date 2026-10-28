---
name: skia/reset-all-members-in-reset-methods
description: When adding member variables, caches, or buffers to reusable classes, ensure they are reset or cleared in all reset and cleanup methods
rubric: correctness
severity: should_fix
---

**Rule.** When introducing or modifying member variables, internal caches, or
buffers in classes with custom reset, clear, or lifecycle cleanup routines,
ensure the new state is cleared or reinitialized in all corresponding reset and
cleanup methods. Do not rely on initial construction alone when a class is
designed to be reused across operations.

**Why.** Classes designed for reuse—such as builders, parsers, collectors, and
context helpers—rely on explicit reset methods to return to a clean baseline.
Omitting newly added member variables or lookup caches from reset routines leaks
stale state across subsequent runs, causing subtle bugs and data corruption that
only manifest on reused instances.

## Examples

Bad:

```cpp
void ParagraphBuilderImpl::Reset() {
    fUTF8IndexForUTF16Index.clear();
    // Bug: omitted newly added fUTF16IndexForUTF8Index lookup cache
    fWordsUtf16.clear();
}
```

Good:

```cpp
void ParagraphBuilderImpl::Reset() {
    fUTF8IndexForUTF16Index.clear();
    fUTF16IndexForUTF8Index.clear();
    fWordsUtf16.clear();
}
```

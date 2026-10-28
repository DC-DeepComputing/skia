---
name: skia/api/attribute-cooperating-class-sentinels-in-comments
description: When documenting fields using sentinel values from cooperating classes, explicitly name the managing class or constant rather than referring vaguely to a special sentinel value
rubric: readability
severity: nit
---

**Rule.** When documenting member fields, pointers, or methods that rely on
sentinel values or special markers assigned by a cooperating class, explicitly
name the managing class (e.g., ResourceCache) or the specific constant instead
of referring vaguely to 'a special sentinel value'. Clarify which component
establishes the sentinel contract and what state it represents.

**Why.** Referring vaguely to 'a special sentinel value' obscures ownership and
forces readers to search across the codebase to deduce which component produces
the value and how it is checked. Explicitly attributing sentinel values to their
managing class clarifies cross-class contracts and aids code navigation.

## Examples

Bad:

```cpp
// A null value means the Resource is not in the return queue. A non-null value means it is
// in the queue, although a special sentinel value is used for the tail address.
Resource* fNextInReturnQueue = nullptr;
```

Good:

```cpp
// A null value means the Resource is not in the return queue. A non-null value means it is in
// the queue, although the ResourceCache assigns a special sentinel value for the tail address.
Resource* fNextInReturnQueue = nullptr;
```

---
name: skia/modules/avoid-transient-parse-pointers-in-persistent-objects
description: When designing persistent objects constructed by parsers or builders, keep transient parse-tree or DOM pointers scoped to builder classes instead of persistent members
rubric: safety
severity: should_fix
---

**Rule.** Do not store raw pointers to transient parse-time data structures
(such as JSON DOM nodes, XML elements, or AST nodes) inside persistent objects
that outlive the parsing phase. Keep transient parser and DOM state scoped to
the builder or parser classes, and extract only persistent data into long-lived
objects.

**Why.** Persistent objects typically outlive the transient data structures
(like skjson::ObjectValue or DOM trees) constructed during parsing. Storing raw
pointers to transient structures in persistent objects leads to dangling
pointers and use-after-free bugs once parsing finishes and the transient DOM is
destroyed.

## Examples

Bad:

```cpp
class SlotManager final : public SkRefCnt {
    ...
    // Handle to "slots" JSON Object, used to grab slot values while building
    const skjson::ObjectValue* fSlotsRoot;
};
```

Good:

```cpp
// In persistent object:
class SlotManager final : public SkRefCnt {
    // Does not retain handles to transient parse DOM
};

// In transient builder (AnimationBuilder / SkottiePriv.h):
class AnimationBuilder {
    ...
    const skjson::ObjectValue* fSlotsRoot;
};
```

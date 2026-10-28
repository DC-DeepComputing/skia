---
name: skia/modules/avoid-composite-types-for-scalar-callback-data
description: When defining public callback or visitor structs, expose only the specific scalar coordinates required by callers rather than broader composite types or temporary testing fields
rubric: design
severity: should_fix
---

**Rule.** When defining public callback or visitor info structs, expose only the
minimal scalar fields (such as advance or trailing position) actually required
by callers. Do not include broader composite types (such as SkRect) when only a
single coordinate is needed, and do not leak temporary testing scaffolding into
public API structures.

**Why.** Exposing composite data structures when callers only consume a single
scalar value overcomplicates API contracts, exposes unneeded layout assumptions,
and risks leaking temporary test state into permanent public interfaces.

## Examples

Bad:

```cpp
struct VisitorInfo {
    const SkFont&   font;
    SkPoint         origin;
    SkRect          rect;  // Unneeded composite type / temporary test scaffolding
    int             count;
};
```

Good:

```cpp
struct VisitorInfo {
    const SkFont&   font;
    SkPoint         origin;
    SkScalar        advanceX;  // Minimal scalar value required by callers
    int             count;
};
```

---
name: skia/avoid-inherited-alias
description: When defining derived classes or qualifying base members, reference concrete base classes explicitly and do not define or retain INHERITED aliases
rubric: readability
severity: should_fix
---

**Rule.** Do not define or use an `INHERITED` type alias (`using INHERITED =
BaseClass;` or `typedef BaseClass INHERITED;`), and remove unused or boilerplate
`INHERITED` aliases. Explicitly qualify base class member calls, overrides, and
constructor initializers with the concrete base class name (e.g.,
`this->BaseClass::method()` or `BaseClass(args)`).

**Why.** Explicitly qualifying base class references clarifies which ancestor in
the inheritance hierarchy is being invoked, adheres to modern Skia style
conventions, and eliminates redundant or dead alias boilerplate from class
declarations.

## Examples

Bad:

```cpp
// Bad: defining INHERITED to qualify base calls, or keeping unused alias boilerplate
class SkCaptureCanvas : public SkNWayCanvas {
    using INHERITED = SkNWayCanvas;
    void willSave() override { this->INHERITED::willSave(); }
};

class SkTypeface_AndroidSystem : public SkTypeface_proxy {
    using INHERITED = SkTypeface_proxy; // unused boilerplate
};
```

Good:

```cpp
// Good: qualify base class directly and omit INHERITED alias
class SkCaptureCanvas : public SkNWayCanvas {
    void willSave() override { this->SkNWayCanvas::willSave(); }
};

class SkTypeface_AndroidSystem : public SkTypeface_proxy {
    // concrete base class qualified directly when needed
};
```

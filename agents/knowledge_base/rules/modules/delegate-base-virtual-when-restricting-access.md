---
name: skia/modules/delegate-base-virtual-when-restricting-access
description: When overriding base class virtual methods as private to restrict external access, delegate to the base class implementation if internal methods rely on base behavior
rubric: correctness
severity: should_fix
---

**Rule.** When overriding a base class virtual method under a private access
specifier to restrict external caller access, ensure the override explicitly
forwards to the base class implementation (e.g., BaseClass::method(...)). Do not
override base virtual methods with empty bodies when internal operations or
dynamic dispatch within the class hierarchy still depend on the base class
functionality.

**Why.** In C++, overriding a virtual method with an empty body displaces the
base class implementation for all dynamic dispatch, including unqualified
internal member calls on this. If internal member functions rely on base
behavior, an empty override silently turns those internal calls into no-ops,
breaking functionality.

## Examples

Bad:

```cpp
class SkCaptureCanvas : public SkNWayCanvas {
private:
    // Bad: Empty overrides displace base virtual dispatch and break internal callers
    void addCanvas(SkCanvas*) override {}
    void removeCanvas(SkCanvas*) override {}
    void removeAll() override {}
};
```

Good:

```cpp
class SkCaptureCanvas : public SkNWayCanvas {
private:
    // Good: Forward to base class to preserve internal behavior while hiding from external callers
    void addCanvas(SkCanvas* canvas) override { SkNWayCanvas::addCanvas(canvas); }
    void removeCanvas(SkCanvas* canvas) override { SkNWayCanvas::removeCanvas(canvas); }
    void removeAll() override { SkNWayCanvas::removeAll(); }
};
```

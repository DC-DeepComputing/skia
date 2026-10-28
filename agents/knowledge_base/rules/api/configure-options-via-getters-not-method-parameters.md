---
name: skia/api/configure-options-via-getters-not-method-parameters
description: When adding configuration flags or migration options to classes or virtual interfaces, use dedicated getters and setters rather than appending optional parameters to operational methods
rubric: design
severity: should_fix
---

**Rule.** Prefer dedicated getter and setter methods on a class or associated
style/options object over adding optional, configuration, or migration
parameters to core operational methods (especially public or virtual
interfaces). Keep operational method signatures stable and minimal, and
configure behavioral toggles and flags through object state instead of threading
arguments through caller stacks and subclass overrides.

**Why.** Adding configuration or temporary migration flags as parameters to
operational methods causes parameter threading across callers and forces every
derived class to update its override signatures. Exposing configuration as
getters and setters on the object or its options struct decouples configuration
from execution, preserves public API stability, and avoids boilerplate parameter
propagation.

## Examples

Bad:

```cpp
// BAD: Adding optional configuration/migration flags as parameters to operational interface methods.
class SK_API Paragraph {
public:
    virtual void layout(SkScalar width, bool applyRoundingHack = true) = 0;
};
```

Good:

```cpp
// GOOD: Configure behavioral options via dedicated getters and setters on the object or style options.
class SK_API ParagraphStyle {
public:
    bool getApplyRoundingHack() const { return fApplyRoundingHack; }
    void setApplyRoundingHack(bool applyRoundingHack) { fApplyRoundingHack = applyRoundingHack; }
private:
    bool fApplyRoundingHack = true;
};

class SK_API Paragraph {
public:
    virtual void layout(SkScalar width) = 0;
};
```

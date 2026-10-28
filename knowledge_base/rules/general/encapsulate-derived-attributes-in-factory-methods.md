---
name: skia/encapsulate-derived-attributes-in-factory-methods
description: When defining factory methods, constructors, or RAII guards operating on an existing object, extract derived state internally instead of accepting redundant caller parameters
rubric: design
severity: should_fix
---

**Rule.** When defining factory methods, constructors, RAII guards, or helper
routines that wrap or operate on an existing object, do not accept redundant
parameters for state or attributes accessible directly through the object
pointer or reference. Extract and query required fields or properties internally
within the helper or constructor implementation rather than requiring callers to
pass duplicate member variables or unpack object state at call sites.

**Why.** Accepting parameters for state already accessible from an underlying
object bloats call sites with unnecessary arguments, breaks encapsulation, and
introduces the risk of callers passing mismatched, stale, or conflicting state.
Reading state directly from the referenced object guarantees consistency and
keeps call sites concise.

## Examples

Bad:

```cpp
// 1. Factory method accepting attributes already accessible from wrapped object:
const SkFontStyle style = realTypeface->fontStyle();
return SkTypeface_Proxy::Make(std::move(realTypeface), style, id);

// 2. RAII guard accepting member field alongside owning object pointer:
class RestoreCurPos {
    RestoreCurPos(Parser* self, const char* curPos) : fSelf(self), fCurPos(curPos) {}
    Parser*     fSelf;
    const char* fCurPos;
};
RestoreCurPos restore(this, fCurPos);
```

Good:

```cpp
// 1. Factory method queries attributes directly from wrapped object:
return SkTypeface_Proxy::Make(std::move(realTypeface), id);
static sk_sp<SkTypeface_Proxy> Make(sk_sp<SkTypeface> realTypeface, TypefaceId id) {
    const SkFontStyle style = realTypeface->fontStyle();
    return sk_sp(new SkTypeface_Proxy(std::move(realTypeface), style, id));
}

// 2. RAII guard reads initial state directly from passed object:
class RestoreCurPos {
    RestoreCurPos(Parser* self) : fSelf(self), fCurPos(self->fCurPos) {}
    Parser*     fSelf;
    const char* fCurPos;
};
RestoreCurPos restore(this);
```

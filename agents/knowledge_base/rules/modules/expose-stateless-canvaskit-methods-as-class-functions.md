---
name: skia/modules/expose-stateless-canvaskit-methods-as-class-functions
description: When binding stateless utility operations in CanvasKit Emscripten bindings, expose static class_function methods on uncounted placeholder classes rather than binding engine classes or instantiable wrappers
rubric: design
severity: should_fix
---

**Rule.** When binding stateless utility operations or calculations in CanvasKit
Emscripten bindings, expose them as static methods using .class_function() on a
namespace placeholder class rather than binding an underlying C++ engine class
directly under an alias or creating instantiable wrapper classes with Make(),
constructors, and .function() instance bindings. Do not inherit placeholder
classes from SkRefCnt or register them with .smart_ptr<sk_sp<T>>(), as clients
never instantiate or manage them. If the operation relies on a shared engine
instance (such as SkUnicode), cache a singleton locally (e.g., via SkOnce)
rather than managing instance lifetimes across JavaScript.

**Why.** Binding an underlying C++ engine class under an aliased JavaScript name
causes Embind type-mapping confusion if that class is referenced elsewhere.
Additionally, stateless calculations require no object lifecycle management in
JavaScript; wrapping them in instantiable classes adds unnecessary boilerplate.
Inheriting placeholder classes from SkRefCnt or registering .smart_ptr
introduces useless ref-counting overhead, vtables, and template bloat for
objects that are never constructed.

## Examples

Bad:

```cpp
// Bad: binding underlying engine class under an alias, or instantiable wrapper with SkRefCnt/smart_ptr
EMSCRIPTEN_BINDINGS(Bidi) {
    class_<SkUnicode>("Bidi")
        .smart_ptr<sk_sp<SkUnicode>>("sk_sp<SkUnicode>")
        .class_function("Make", optional_override([]() { return SkUnicodes::Bidi::Make(); }))
        .function("_getBidiRegions", ...);
}

class BidiPlaceholder : public SkRefCnt { };
EMSCRIPTEN_BINDINGS(Bidi) {
    class_<BidiPlaceholder>("Bidi")
        .smart_ptr<sk_sp<BidiPlaceholder>>("sk_sp<BidiPlaceholder>")
        .class_function("_getBidiRegions", ...);
}
```

Good:

```cpp
// Good: uncounted placeholder class without SkRefCnt/smart_ptr, backed by a cached engine singleton
class BidiPlaceholder { };

static sk_sp<SkUnicode> getUnicode() {
    static sk_sp<SkUnicode> unicode;
    static SkOnce once;
    once([] { unicode = SkUnicodes::Bidi::Make(); });
    return unicode;
}

EMSCRIPTEN_BINDINGS(Bidi) {
    class_<BidiPlaceholder>("Bidi")
        .class_function("_getBidiRegions", optional_override([](JSString text) {
            return getUnicode()->getBidiRegions(...);
        }));
}
```

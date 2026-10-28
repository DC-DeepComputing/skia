---
name: skia/api/return-base-class-from-interface-factories
description: When declaring factory or interface methods, return root base interfaces rather than derived types or intermediate proxy classes to avoid leaking implementation details
rubric: readability
severity: should_fix
---

**Rule.** Return root base class or interface pointers (or smart pointers) from
factory and interface methods rather than concrete derived types when callers
only interact with the base interface, avoiding unnecessary #include
dependencies on derived headers. When implementing classes that inherit from
intermediate helper or proxy base classes (such as SkTypeface_proxy), do not
return the intermediate base class from static Make factories; return either the
polymorphic root interface (e.g., sk_sp<SkTypeface>) or the concrete derived
type itself.

**Why.** Returning concrete derived class pointers from polymorphic or factory
methods forces callers to include internal implementation headers, causing
header dependencies and build cascades. Returning intermediate helper or proxy
base classes leaks internal adapter inheritance structure to callers who only
need the polymorphic root interface or the concrete subclass's interface.

## Examples

Bad:

```cpp
// Leaking intermediate proxy base from static Make:
class SkTypeface_fontconfig : public SkTypeface_proxy {
public:
    static sk_sp<SkTypeface_proxy> Make(SkAutoFcPattern pattern,
                                        SkString sysroot,
                                        SkFontScanner* scanner);
};

// Returning concrete derived pointer forces derived #include:
virtual SkCaptureCanvas* makeCaptureCanvas(SkCanvas*) = 0;
```

Good:

```cpp
// Returning polymorphic root interface from static Make:
class SkTypeface_fontconfig : public SkTypeface_proxy {
public:
    static sk_sp<SkTypeface> Make(SkAutoFcPattern pattern,
                                  SkString sysroot,
                                  SkFontScanner* scanner);
};

// Returning base interface pointer decouples callers:
virtual SkCanvas* makeCaptureCanvas(SkCanvas*) = 0;
```

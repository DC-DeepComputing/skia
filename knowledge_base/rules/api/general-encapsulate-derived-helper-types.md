---
name: skia/api/general-encapsulate-derived-helper-types
description: When declaring class inheritance hierarchies or abstract base interfaces, keep implementation- or backend-specific helper methods, structs, and nested types in concrete derived classes rather than base classes
rubric: design
severity: should_fix
---

**Rule.** Keep implementation-specific helper structs, nested types, and
backend-specific helper methods or data formats encapsulated within the concrete
derived classes or backend utility classes that use them. Do not declare
implementation-specific helpers or methods in an abstract base class or generic
interface (such as DrawListBase or SkFontScanner), even under protected: or as
static utilities, when they only serve a specific concrete backend or subclass.

**Why.** Placing implementation- or backend-specific helper types and methods in
base classes pollutes generic abstractions with details irrelevant to other
subclasses, couples portable interfaces to specific libraries or hardware
backends, and burdens alternative implementations with single-backend
idiosyncrasies.

## Examples

Bad:

```cpp
// Bad: Helper struct in base class that only serves a concrete subclass
class DrawListBase {
protected:
    struct Draw {
        const Renderer* fRenderer;
        DrawParams fDrawParams;
    };
};

// Bad: Backend-specific helper method on generic interface
class SkFontScanner : public SkNoncopyable {
public:
    virtual bool scanFont(...) const = 0;
    static void computeAxisValues(AxisDefinitions axisDefs, SkFixed* axisValues);
};
```

Good:

```cpp
// Good: Encapsulate helper struct in the derived class that uses it
class DrawList : public DrawListBase {
private:
    struct Draw {
        const Renderer* fRenderer;
        DrawParams fDrawParams;
    };
};

// Good: Scope backend-specific helper to concrete backend subclass
class FontScanner_FreeType : public SkFontScanner {
public:
    static void computeAxisValues(AxisDefinitions axisDefs, SkFixed* axisValues);
};
```

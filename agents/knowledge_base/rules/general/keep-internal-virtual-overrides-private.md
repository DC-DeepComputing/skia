---
name: skia/keep-internal-virtual-overrides-private
description: When declaring class helpers, internal types, member fields, or virtual overrides, keep them private rather than exposing them publicly for implementation helpers
rubric: design
severity: should_fix
---

**Rule.** Declare internal helper methods, member fields, internal types (enums
or structs), and virtual overrides as private rather than public or protected
unless derived classes must call the base implementation. Do not make
class-internal types public merely to allow implementation helpers to use them;
declare the helpers as private static member functions or move both into the
.cpp anonymous namespace. In leaf or TU-local classes, avoid protected sections
and keep non-public members private. When a virtual hook uses a nested type in
its signature that subclasses must name, declare that type protected.

**Why.** Exposing internal helper methods, member fields, types, or virtual
overrides in public interfaces violates encapsulation, breaks modular layering,
and expands public API commitments. Keeping internal types private and defining
implementation helpers as private static methods or TU-local anonymous functions
preserves encapsulation without leaking implementation details.

## Examples

Bad:

```cpp
// Bad: Making internal enum public solely for a .cpp free function helper
class WGSLCodeGenerator : public CodeGenerator {
public:
    enum class PtrAddressSpace { kFunction, kPrivate, kStorage };
};
// In .cpp:
std::string_view address_space_to_str(WGSLCodeGenerator::PtrAddressSpace space);

// Bad: Exposing internal virtual override in public section of derived class
class SkGaneshRecorder final : public SkRecorder {
public:
    SkCanvas* makeCaptureCanvas(SkCanvas*) override;
};
```

Good:

```cpp
// Good: Keep internal enum private and helper as private static member
class WGSLCodeGenerator : public CodeGenerator {
private:
    enum class PtrAddressSpace { kFunction, kPrivate, kStorage };
    static std::string_view AddressSpaceToStr(PtrAddressSpace);
};

// Good: Keep virtual overrides and internal members private
class SkGaneshRecorder final : public SkRecorder {
private:
    SkCanvas* makeCaptureCanvas(SkCanvas*) override;
};
```

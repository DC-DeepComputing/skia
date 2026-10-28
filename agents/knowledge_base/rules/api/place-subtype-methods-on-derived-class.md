---
name: skia/api/place-subtype-methods-on-derived-class
description: When introducing specialized subtypes or operations, place domain methods, role-specific operations, and conversion operators on concrete derived classes rather than generic base classes
rubric: design
severity: should_fix
---

**Rule.** Place subtype-specific methods, role-specific operations (such as root
container rendering), and domain-specific conversion operators directly on
concrete derived classes rather than on generic base classes or utility types.
Do not pollute generic base classes with operations meaningful only for a
specific subtype or role, rely on runtime assertions (such as isRasterBacked()),
or allow unconverted types to compile silently. Additionally, concrete factory
functions should return smart pointers to the derived type (e.g.,
sk_sp<Derived>) instead of the base type to avoid downcasting at call sites.

**Why.** Exposing specialized operations, role-specific methods, or conversion
operators on generic base classes pollutes the base interface, exposes
meaningless operations on sibling classes, weakens type safety, and risks C++
method name hiding. Placing operations directly on concrete derived classes
leverages compile-time checking, prevents invalid usage on unrelated siblings,
and eliminates downcasting at call sites.

## Examples

Bad:

```cpp
// Variant 1: Domain-specific conversion operators left on generic base class
class VectorValue : public std::vector<float> {
public:
    operator SkV3()      const;
    operator SkColor()   const;  // Domain-specific to ColorValue
    operator SkColor4f() const;
};
class ColorValue final : public VectorValue {};

// Variant 2: Role-specific method placed on generic base class instead of derived type
class SkSVGNode : public SkRefCnt {
public:
    void render(const SkSVGRenderContext&) const;
    void render(const SkSVGRenderContext&, SkSVGNode&) const; // Specific to root container
};

// Variant 3: Subtype operation on base/utility class with runtime assertion
sk_sp<SkShader> SkImageShader::MakeForPaint(sk_sp<SkImage> image, const SkPaint& paint) {
    SkASSERT(as_IB(image)->isRasterBacked());
    ...
}
```

Good:

```cpp
// Variant 1: Domain-specific conversion operators moved to specialized subtype
class VectorValue : public std::vector<float> {
public:
    operator SkV3()      const;
};
class ColorValue final : public VectorValue {
public:
    ColorValue() = default;
    operator SkColor()   const;
    operator SkColor4f() const;
};

// Variant 2: Role-specific method placed directly on concrete derived class
class SkSVGNode : public SkRefCnt {
public:
    void render(const SkSVGRenderContext&) const;
};
class SkSVGSVG : public SkSVGContainer {
public:
    void renderNode(const SkSVGRenderContext&, const SkSVGIRI& iri) const;
};

// Variant 3: Method on concrete derived class; factory returns concrete type
static sk_sp<SkImage_Raster> MakeFromBitmap(const SkBitmap&, SkCopyPixelsMode);
sk_sp<SkShader> makeShaderForPaint(const SkPaint& paint);
```

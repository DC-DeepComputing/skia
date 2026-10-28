---
name: skia/avoid-speculative-virtual-destructors
description: When defining classes or interfaces, avoid virtual methods, destructors, or abstract interfaces unless runtime polymorphic dispatch or subclass customization is concretely required
rubric: design
severity: should_fix
---

**Rule.** Do not speculatively declare member functions, destructors, or
subclass customization hooks as virtual until polymorphic dispatch or subclass
customization is concretely required. On concrete classes or standalone adapters
without subclasses, keep methods and destructors strictly non-virtual. In base
classes, keep accessors non-virtual and provide non-virtual methods when the
base implementation suffices. Furthermore, prefer plain data structs or direct
property passing over abstract base classes with virtual methods when
representing fixed objects or static configuration.

**Why.** Declaring virtual functions or introducing abstract interfaces when
polymorphism is not needed incurs dynamic dispatch overhead, adds unnecessary
vtable pointer (vptr) bloat, inhibits compiler inlining, and misleads readers
into expecting subclass customization points for state that never changes
dynamically.

## Examples

Bad:

```cpp
// Bad: Declaring virtual methods or destructors on a non-polymorphic concrete class
class SkStreamAdapter {
public:
    virtual ~SkStreamAdapter() = default;
    virtual size_t read(rust::Slice<uint8_t> buffer);
};

// Bad: Introducing an abstract interface with virtual methods for fixed configuration
class Renderers {
public:
    virtual const skgpu::graphite::Renderer* bitmapText() const = 0;
    virtual const skgpu::graphite::Renderer* sdfText(bool useLCDText) const = 0;
    virtual ~Renderers() {}
};
```

Good:

```cpp
// Good: Keep concrete classes and adapters non-virtual
class SkStreamAdapter {
public:
    ~SkStreamAdapter() = default;
    size_t read(rust::Slice<uint8_t> buffer);
};

// Good: Use a plain data struct or direct property passing for fixed configuration
struct RendererData {
    bool isSDF = false;
    bool isLCD = false;
};
```

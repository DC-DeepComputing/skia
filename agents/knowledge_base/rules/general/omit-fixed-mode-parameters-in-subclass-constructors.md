---
name: skia/omit-fixed-mode-parameters-in-subclass-constructors
description: When specialized subclasses, class templates, or helpers have invariant modes, initialize base classes directly or via static constexpr traits and assert matching context state
rubric: design
severity: should_fix
---

**Rule.** When defining specialized concrete subclasses, class templates
parameterized on a type family, or helper methods that inherently represent an
invariant mode or type tag, do not accept mode or discriminator parameters in
constructors or helper signatures. For specialized subclasses, pass constant
flags directly to the base constructor in member initializers and assert with
SkASSERT that the context object matches the expected state; for class templates
parameterized by T, define a static constexpr trait member on T and forward
T::kTrait to the base class; for helper methods, omit invariant parameters and
configure state internally.

**Why.** Accepting mode, state, or discriminator parameters when values are
invariant or statically determined by a template argument introduces unnecessary
configurability, clutters call sites with boilerplate arguments or branches, and
risks passing mismatched flags. Initializing base classes directly simplifies
signatures with zero per-instance overhead, while asserting expected state in
base constructors verifies that the target context is configured for that
operational mode.

## Examples

Bad:

```cpp
// Specialized subclass accepting redundant mode parameter
class DrawWriter::Vertices : private DrawWriter::Appender {
public:
    Vertices(DrawWriter& w, SkEnumBitMask<RenderStateFlags> newRenderState)
            : Appender(w, newRenderState) {
        w.setTemplate(w.fVertices, {}, {}, 0, newRenderState);
    }
};

// Class template accepting runtime parameter for type tag known from T
template <typename T>
struct BindingList : public BindingWrapper {
    BindingList(BindingListType type) : BindingWrapper(type) {}
};

// Helper method accepting invariant mode parameter
void draw(BindBufferInfo vertices, unsigned int vertexCount,
          SkEnumBitMask<RenderStateFlags> renderState) {
    this->bindAndFlush(vertices, {}, {}, 0, vertexCount, renderState);
}
```

Good:

```cpp
// Pass invariant constant directly to base constructor and assert context state
class DrawWriter::Vertices : private DrawWriter::Appender {
public:
    Vertices(DrawWriter& w)
            : Appender(w, RenderStateFlags::kAppendVertices) {
        w.setTemplate(w.fVertices, {}, {}, 0);
    }
};
Appender::Appender(DrawWriter& w, SkEnumBitMask<RenderStateFlags> renderState)
        : fDrawer(w) {
    SkASSERT(w.fRenderState == renderState);
}

// Forward static constexpr trait member directly from template argument T
template <typename T>
struct BindingList : public BindingWrapper {
    BindingList() : BindingWrapper(T::kListType) {}
};

// Omit parameter and handle invariant mode internally
void draw(BindBufferInfo vertices, unsigned int vertexCount) {
    this->bindAndFlush(vertices, {}, {}, 0, vertexCount);
}
```

---
name: skia/api/use-non-virtual-interface-pattern
description: When designing class hierarchies, keep virtual onMethod hooks protected or private; avoid public virtuals and route internal access through friended *Priv helpers
rubric: readability
severity: should_fix
---

**Rule.** When designing extensible class hierarchies, follow the Non-Virtual
Interface (NVI) pattern: expose non-virtual public entry points delegating to
private or protected virtual on<MethodName>() hooks. Never expose virtual on*
hooks in public class interfaces; keep them protected or private, and route
internal or platform access through a friended *Priv helper class in src/. Do
not make public entry points virtual or split operations across multiple virtual
lifecycle hooks; provide a single virtual customization point and expose
protected helpers for sub-steps instead. When an intermediate class must enforce
common logic across subclasses, override the base on<MethodName>() as final and
expose a dedicated hook for subclasses. Conversely, never prefix non-virtual
methods with 'on'; reserve on* exclusively for polymorphic hooks.

**Why.** Exposing virtual on* hooks on public interfaces violates NVI
encapsulation and pollutes client-facing headers with customization hooks;
keeping hooks protected and routing internal access through friended *Priv
helpers preserves clean public contracts. Relying on derived classes to
explicitly invoke superclass methods risks bypassing base invariants, while
overriding intermediate hooks as final guarantees execution order. Splitting
operations across multiple virtual lifecycle hooks introduces unnecessary
virtual interface surface; a single virtual customization point delegating
sub-steps to protected helpers keeps interfaces cohesive. Furthermore, prefixing
non-virtual methods with 'on' misleadingly implies polymorphic dispatch.

## Examples

Bad:

```cpp
// Bad: Exposing virtual on* customization hooks in public class interfaces
class SkTypeface {
public:
    virtual void* onGetDWriteFontFace() const { return nullptr; }
};

// Bad: Fragmenting an operation across multiple virtual lifecycle hooks
class SkCodec {
public:
    bool rewindIfNeeded() { return this->attemptRewind() && this->onRewind(); }
protected:
    virtual bool attemptRewind() { return !fStream || fStream->rewind(); }
    virtual bool onRewind() = 0;
};

// Bad: Virtual public entry points and 'on' prefix on non-virtual methods
class SkSVGNode {
public:
    virtual SkRect objectBoundingBox(const SkSVGRenderContext&) const;
    void onPipelineCompileRequired(); // Non-virtual method should not use 'on'
};
```

Good:

```cpp
// Good: Protected virtual on* hook with internal caller access routed via friended *Priv helper
class SkTypeface {
protected:
    virtual void* onGetDWriteFontFace() const { return nullptr; }
    friend class SkFontPriv;
};
class SkFontPriv {
public:
    static void* SkTypeface_GetIDWriteFontFace(const SkTypeface* tf) {
        return tf->onGetDWriteFontFace();
    }
};

// Good: Single virtual customization point delegating sub-steps to protected helpers
class SkCodec {
public:
    bool rewindIfNeeded() { return this->onRewind(); }
protected:
    bool rewindStream() { return !fStream || fStream->rewind(); }
    virtual bool onRewind() { return this->rewindStream(); }
};

// Good: Public non-virtual NVI entry point, final intermediate hook, descriptive event name
class SkSVGNode {
public:
    SkRect objectBoundingBox(const SkSVGRenderContext& ctx) const {
        return this->onObjectBoundingBox(ctx);
    }
    void pipelineCompileWasRequired(); // Descriptive past-tense for non-virtual event
protected:
    virtual SkRect onObjectBoundingBox(const SkSVGRenderContext& ctx) const = 0;
};
```

---
name: skia/inline-trivial-accessors-in-headers
description: 'When defining class methods or constructors, inline trivial accessors in headers, but move non-trivial logic, virtual methods, overrides, and #ifdef branches to .cpp files'
rubric: readability
severity: should_fix
---

**Rule.** Define trivial one-line accessors, mutators, and helper methods (such
as field getters, boolean state queries, or simple container appenders) inline
directly within the class declaration in the header, provided doing so does not
require adding new #include dependencies instead of forward declarations.
Conversely, do not define non-trivial methods, constructors, virtual functions
(even empty `{}` default implementations), overrides, or methods with
conditional compilation (#ifdef) branches (particularly those containing loops,
branching fallback logic, multi-line initialization, debug assertions, backend
or configuration #ifdef blocks, synchronization locks, or stream operations)
inline in header files; declare them in the header and define them out-of-line
in the corresponding .cpp source file.

**Why.** Inlining simple getters, setters, and one-line helpers inside the
header declaration eliminates boilerplate out-of-line definitions in source
files and enables compiler optimizations across translation units. Conversely,
inlining non-trivial methods, complex constructors, virtual methods or overrides
(which implicitly request inlining on methods dispatched dynamically), or #ifdef
branches bloats translation units, leaks internal preprocessor guards or backend
implementation details into public/shared headers, forces heavy include
dependencies, and degrades incremental build times.

## Examples

Bad:

```cpp
// Foo.h / Foo.cpp - Bad: trivial accessor or one-line helper defined out-of-line
class Foo {
public:
    bool hasActivePassEncoder() const;
    void trackLayerInfo(const LayerInfo& info) const;
};
void Foo::trackLayerInfo(const LayerInfo& info) const {
    fLayerInfo.emplace_back(info);
}

// GlobalCache.h - Bad: non-trivial method with loop/lock defined inline in header
const Sampler* getDynamicSampler(SamplerDesc desc) const SK_EXCLUDES(fSpinLock) {
    SkAutoSpinlock lock{fSpinLock};
    for (uint32_t i = 0; i < kNumDynamicSamplers; ++i) { ... }
}

// SkottieProperty.h - Bad: virtual method defined inline in header
class PropertyObserver : public SkRefCnt {
public:
    virtual void onAnimationBuilt() {}
};
```

Good:

```cpp
// Foo.h - Good: trivial accessor and one-line helper inlined in header declaration
class Foo {
public:
    bool hasActivePassEncoder() const {
        return fActiveRenderPassEncoder || fActiveComputePassEncoder;
    }
    void trackLayerInfo(const LayerInfo& info) const { fLayerInfo.emplace_back(info); }
};

// GlobalCache.h / .cpp - Good: declared in header, defined in .cpp
const Sampler* getDynamicSampler(SamplerDesc desc) const SK_EXCLUDES(fSpinLock);

// SkottieProperty.h / .cpp - Good: virtual method declared in header, defined in .cpp
// In SkottieProperty.h:
class PropertyObserver : public SkRefCnt {
public:
    virtual void onAnimationBuilt();
};
// In SkottieProperty.cpp:
void PropertyObserver::onAnimationBuilt() {}
```

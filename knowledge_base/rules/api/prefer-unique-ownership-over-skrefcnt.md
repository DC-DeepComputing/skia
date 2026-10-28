---
name: skia/api/prefer-unique-ownership-over-skrefcnt
description: When designing utility interfaces, caches, or backend contexts without shared ownership, prefer std::unique_ptr and SkNoncopyable over inheriting SkRefCnt
rubric: design
severity: should_fix
---

**Rule.** Do not make classes or interfaces inherit from SkRefCnt (or
SkNVRefCnt) and require sk_sp management unless genuine shared ownership across
multiple contexts or threads is necessary. For modern backend Context and
Recorder classes (such as SkRasterContext or SkRecorder), follow Graphite's
architecture where contexts and recorders have exclusive ownership managed via
std::unique_ptr rather than legacy Ganesh-style reference counting. Similarly,
for standalone caches, managers, and polymorphic utility interfaces (such as
SkFontScanner), prefer unique ownership via std::unique_ptr, borrow via
non-owning raw pointers or references, disallow copying (e.g. by inheriting
SkNoncopyable), and declare a virtual destructor.

**Why.** In modern Skia architecture, backend contexts, recorders, and utility
interfaces are designed with exclusive ownership, aligning new components with
Graphite's decoupled model rather than legacy Ganesh's shared reference
counting. Moreover, reference counting introduces atomic increment/decrement
overhead, obscures single-owner lifetime semantics, and misleads callers into
expecting shared ownership. For polymorphic single-owner interfaces,
SkNoncopyable and a virtual destructor prevent object slicing and undefined
deletion behavior.

## Examples

Bad:

```cpp
// Bad: Deriving modern backend context from SkRefCnt with sk_sp factory
class SK_API SkRasterContext : public SkRefCnt {
public:
    static sk_sp<const SkRasterContext> Make(const Options&);
};

// Bad: Using SkRefCnt for single-owner utility interface or cache
class SkFontScanner : public SkRefCnt {
public:
    virtual bool scanFont(SkStreamAsset* stream, ...) const = 0;
};
class SkResourceCache : public SkRefCnt {
public:
    static sk_sp<SkResourceCache> Make(size_t byteLimit);
};
```

Good:

```cpp
// Good: Modern backend context with exclusive ownership via std::unique_ptr
class SK_API SkRasterContext {
public:
    static std::unique_ptr<const SkRasterContext> Make(const Options&);
};

// Good: Utility interface with unique ownership, non-copyable semantics, and virtual dtor
class SkFontScanner : public SkNoncopyable {
public:
    virtual ~SkFontScanner() = default;
    virtual bool scanFont(SkStreamAsset* stream, ...) const = 0;
};
// Single owner stores std::unique_ptr:
std::unique_ptr<SkFontScanner> fScanner;

// Good: Direct value or std::unique_ptr ownership for single-owner cache
class SkResourceCache {
public:
    explicit SkResourceCache(size_t byteLimit);
};
```

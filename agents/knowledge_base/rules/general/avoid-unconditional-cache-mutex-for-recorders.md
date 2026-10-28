---
name: skia/avoid-unconditional-cache-mutex-for-recorders
description: When designing caches or data structures, keep base instances unsynchronized and decouple concurrency into separate wrappers, derived classes, or Context-level locks
rubric: design
severity: should_fix
---

**Rule.** Do not embed a mutex into general-purpose data structures or cache
classes to make instances synchronized by default. Keep base cache and container
classes unsynchronized for single-threaded efficiency, and factor
synchronization into a separate synchronized wrapper, derived subclass, or
external Context-level lock (such as per-Recorder caches shared at the Context
level).

**Why.** Embedding a mutex into core data structures imposes locking overhead
and memory bloat on single-threaded use cases, while rarely providing sufficient
atomicity for composite multi-step operations. Factoring synchronization into a
wrapper or derived subclass adheres to the zero-overhead principle and cleanly
decouples concurrency control from core container logic.

## Examples

Bad:

```cpp
// Bad: Embedding a mutex in the base cache forces locking overhead on single-threaded callers.
class SkResourceCache {
public:
    bool find(const Key& key, FindVisitor visitor, void* context) {
        SkAutoMutexExclusive am(fMutex);
        return this->findInternal(key, visitor, context);
    }
    void add(Rec* rec, void* payload = nullptr);
private:
    mutable SkMutex fMutex;
};
```

Good:

```cpp
// Good: Base cache is unsynchronized for single-threaded or per-Recorder usage.
class SkResourceCache {
public:
    virtual bool find(const Key& key, FindVisitor visitor, void* context);
    virtual void add(Rec* rec, void* payload = nullptr);
};

// Concurrency is factored into an explicit subclass or handled at Context level.
class SkSynchronizedResourceCache : public SkResourceCache {
public:
    bool find(const Key& key, FindVisitor visitor, void* context) override {
        SkAutoMutexExclusive am(fMutex);
        return SkResourceCache::find(key, visitor, context);
    }
private:
    mutable SkMutex fMutex;
};
```

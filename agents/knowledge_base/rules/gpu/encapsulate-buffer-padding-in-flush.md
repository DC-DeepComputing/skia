---
name: skia/gpu/encapsulate-buffer-padding-in-flush
description: When designing batching buffer writers that require tail alignment padding, encapsulate cleanup within the public flush() method rather than exposing separate cleanup helpers
rubric: design
severity: should_fix
---

**Rule.** Encapsulate buffer alignment padding and tail-data cleanup inside the
public flush() method of batching writers rather than exposing separate public
cleanup helpers (such as padAndZero()) that external callers must invoke before
flushing. Provide a private or internal flush helper (such as flushInternal())
for internal buffer reallocation routines that manage chunk transitions
separately.

**Why.** Exposing buffer padding or alignment cleanup as a separate public
method forces external callers (such as pipeline switches or pass finalizers) to
manually coordinate cleanup before flushing, risking missed edge cases where
trailing padding remains uninitialized. Encapsulating cleanup inside public
flush() guarantees alignment invariants are upheld at every batch boundary while
keeping low-level buffer transitions isolated.

## Examples

Bad:

```cpp
// Bad: Exposing padAndZero() publicly forces callers to manually coordinate cleanup:
class DrawWriter {
public:
    void padAndZero();
    void flush();
};

drawWriter.padAndZero();
drawWriter.flush();
```

Good:

```cpp
// Good: Encapsulate padding inside public flush() and keep internal flush private:
class DrawWriter {
public:
    void flush() {
        this->padAndZero();
        this->flushInternal();
    }
private:
    void padAndZero();
    void flushInternal();
};

drawWriter.flush();
```

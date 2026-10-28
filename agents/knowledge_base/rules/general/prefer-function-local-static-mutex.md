---
name: skia/prefer-function-local-static-mutex
description: When a static mutex or spinlock synchronizes only a single method, declare it as a function-local static instead of a class member
rubric: readability
severity: should_fix
---

**Rule.** When a static mutex or spinlock (such as SkMutex or SkSpinlock) is
only used to synchronize execution of a single function or method, declare it as
a function-local static variable inside that function rather than as a static
data member of the class. Move any mutex headers out of the class header into
the .cpp implementation file.

**Why.** Scoping the static mutex inside the function that uses it encapsulates
synchronization logic to where it is needed, avoids exposing internal locks to
other class methods, keeps class declarations clean, and eliminates unnecessary
header include dependencies.

## Examples

Bad:

```cpp
// In header:
class SkAvifCodec : public SkScalingCodec {
    ...
private:
    static SkMutex fMutex;
};

// In .cpp:
SkMutex SkAvifCodec::fMutex;

SkCodec::Result SkAvifCodec::onGetPixels(...) {
    SkAutoMutexExclusive lock(fMutex);
    return this->decodePixels(...);
}
```

Good:

```cpp
// In header: class definition requires no static mutex member or mutex header include

// In .cpp:
SkCodec::Result SkAvifCodec::onGetPixels(...) {
    static SkMutex mutex;
    SkAutoMutexExclusive lock(mutex);
    return this->decodePixels(...);
}
```

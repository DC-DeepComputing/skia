---
name: skia/api/document-mutex-guarded-state
description: When declaring class mutexes, spinlocks, or lock members, document what shared resources, state, or operations they guard or use SK_GUARDED_BY
rubric: readability
severity: nit
---

**Rule.** When declaring a mutex or lock member variable (such as SkMutex,
SkSpinlock, or std::mutex), document in an explanatory comment which shared
resources, critical sections, or operations the lock protects. If the lock
guards specific member variables, annotate those fields with SK_GUARDED_BY or
explicitly identify them in the lock's doc comment.

**Why.** Synchronization primitives declared without explanatory comments or
annotations leave thread-safety boundaries unclear, making it difficult for
maintainers to determine which methods require locking and risking data races or
deadlocks during maintenance.

## Examples

Bad:

```cpp
class SkCrabbyAvifCodec : public SkScalingCodec {
    ...
private:
    static SkMutex fMutex;
    SkCodecImage fImage;
};
```

Good:

```cpp
class SkCrabbyAvifCodec : public SkScalingCodec {
    ...
private:
    // Guards access to onGetPixels to ensure only one decoder instance is created at a time.
    static SkMutex fMutex;
    SkCodecImage fImage SK_GUARDED_BY(fMutex);
};
```

---
name: skia/initialize-local-singletons-at-declaration
description: When defining function-local statics with non-trivial destructors, wrap them in SkNoDestructor or leaked pointers and initialize directly without SkOnce
rubric: readability
severity: blocking
---

**Rule.** Do not store function-local statics or singletons in bare types with
non-trivial destructors (such as static sk_sp<T>, std::string, or containers),
nor use manual SkOnce, std::call_once, or unsynchronized if (!ptr) guards.
Instead, wrap non-trivially destructible statics in SkNoDestructor<T> (or store
an intentionally leaked static raw pointer static T* gSingleton = ...release()),
and initialize directly at declaration (using an immediately invoked lambda if
multi-step initialization is required), relying on C++11 thread-safe static
initialization.

**Why.** In C++11 and later, function-local static variables are guaranteed by
the standard to be initialized in a thread-safe manner on first execution
('magic statics'), making SkOnce or manual checks redundant boilerplate.
Furthermore, static variables with non-trivial destructors register exit-time
destructors (__cxa_atexit), increasing binary/runtime footprint and risking
shutdown use-after-free bugs; SkNoDestructor<T> and intentionally leaked
pointers prevent exit-time destruction.

## Examples

Bad:

```cpp
// Bad: static sk_sp registers an exit-time destructor, risking shutdown UAF.
sk_sp<SkPathData> SkPathData::Empty() {
    static sk_sp<SkPathData> gEmpty = MakeNoCheck({}, {}, {}, {}, {});
    return gEmpty;
}

// Bad: redundant SkOnce and bare static sk_sp with non-trivial destructor.
sk_sp<SkFontMgr> SkFontMgr::RefDefault() {
    static SkOnce once;
    static sk_sp<SkFontMgr> singleton;
    once([] {
        sk_sp<SkFontMgr> fm = gSkFontMgr_DefaultFactory ? gSkFontMgr_DefaultFactory()
                                                        : SkFontMgr::Factory();
        singleton = fm ? std::move(fm) : RefEmpty();
    });
    return singleton;
}

// Bad: unsynchronized lazy check creates a data race.
SkRasterRecorder* SkRasterRecorder::Get() {
    static SkRasterRecorder* gRecorder = nullptr;
    if (!gRecorder) { gRecorder = MakeRecorder().release(); }
    return gRecorder;
}
```

Good:

```cpp
// Good: SkNoDestructor prevents exit-time destructor registration;
// immediately invoked lambda initializes thread-safely at declaration without SkOnce.
sk_sp<SkFontMgr> SkFontMgr::RefDefault() {
    static SkNoDestructor<sk_sp<SkFontMgr>> singleton([] {
        sk_sp<SkFontMgr> fm = gSkFontMgr_DefaultFactory ? gSkFontMgr_DefaultFactory()
                                                        : SkFontMgr::Factory();
        return fm ? std::move(fm) : RefEmpty();
    }());
    return *singleton;
}

// Good: magic statics ensure thread-safe one-time initialization;
// raw pointer avoids exit-time destructor registration.
sk_sp<SkPathData> SkPathData::Empty() {
    static SkPathData* gEmpty = MakeNoCheck({}, {}, {}, {}, {}).release();
    return sk_ref_sp(gEmpty);
}

SkRasterRecorder* SkRasterRecorder::Get() {
    static SkRasterRecorder* gRecorder = MakeRecorder().release();
    return gRecorder;
}
```

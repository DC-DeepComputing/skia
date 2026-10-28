---
name: skia/api/prefix-sk-sp-accessors-with-ref
description: When defining ref-counted accessors, return borrowed pointers or persistent const sk_sp<T>& (never referencing temporaries or nullptr), and reserve ref prefixes for owning sk_sp getters
rubric: safety
severity: blocking
---

**Rule.** When defining accessors for retained ref-counted objects or buffers,
return a borrowed raw pointer (T*, const void*) or const sk_sp<T>& rather than
sk_sp<T> by value to avoid atomic ref-count churn. Retain borrowed accessors
alongside owning getters (e.g. uniforms() alongside refUniforms()). Only return
const sk_sp<T>& when referencing an existing persistent member of identical
type; never return const sk_sp<T>& to nullptr, computed lookups, or upcast base
types (e.g. const sk_sp<Base>& from sk_sp<Derived>), which dangles a temporary.
Never call .release() in getters, and reserve ref prefixes for owning copies.

**Why.** Returning sk_sp<T> by value incurs atomic ref-count churn, while
calling .release() drops ownership and empties the member. Providing borrowed
raw-pointer accessors allows performance-sensitive consumers to access data with
zero overhead. However, returning const sk_sp<T>& when yielding nullptr,
computed results, or upcast base types (sk_sp<Derived>) binds the reference to a
compiler-generated temporary that is immediately destroyed, causing undefined
behavior from dangling references. Reserving ref-prefixed names clarifies
ownership transfer.

## Examples

Bad:

```cpp
class Context {
public:
    // Bad: plain getters return owning sk_sp by value, causing ref-count churn.
    sk_sp<SkCaptureManager> captureManager() const { return fCaptureManager; }
    sk_sp<SkColorSpace> colorSpace() const { return fColorSpace; }

    // Bad: returning nullptr constructs a temporary sk_sp; returning const& dangles.
    const sk_sp<skresources::ImageAsset>& getImage(SlotID id) const { return nullptr; }

    // Bad: returning const sk_sp<Base>& when holding sk_sp<Derived> dangles a temporary.
    const sk_sp<ResourceProvider>& getProvider() const { return fSlottableProvider; }

    // Bad: 'ref' prefix used for an accessor returning a borrowed raw pointer.
    SkShader* refShader() const { return fShader.get(); }

    // Bad: calling .release() in a getter empties member state and drops ownership.
    SlotManager* getSlotManager() { return fSlotManager.release(); }
private:
    sk_sp<SkCaptureManager> fCaptureManager;
    sk_sp<SkColorSpace> fColorSpace;
    sk_sp<SlottableResourceProvider> fSlottableProvider;
    sk_sp<SkShader> fShader;
    sk_sp<SlotManager> fSlotManager;
};

// Bad: getter returns sk_sp by value even though the singleton/provider retains ownership.
SK_API sk_sp<Factory> GetFactory();
```

Good:

```cpp
class Context {
public:
    // Good: plain getters return borrowed raw pointer or const ref without ref churn.
    SkCaptureManager* captureManager() const { return fCaptureManager.get(); }
    const sk_sp<SkColorSpace>& colorSpace() const { return fColorSpace; }

    // Good: return sk_sp by value when returning nullptr or fallible/computed lookups.
    sk_sp<const skresources::ImageAsset> getImage(SlotID id) const {
        return fImageMap.empty() ? nullptr : fImageMap.front();
    }

    // Good: return borrowed Base* (or sk_sp<Base> by value) when upcasting derived member.
    ResourceProvider* getProvider() const { return fSlottableProvider.get(); }

    // Good: 'ref' prefix explicitly signals that an owning sk_sp is returned (+1 ref).
    sk_sp<SkShader> refShader() const { return fShader; }

    // Good: returns const sk_sp& to share ref-counted handle without emptying member.
    const sk_sp<SlotManager>& getSlotManager() const { return fSlotManager; }
private:
    sk_sp<SkCaptureManager> fCaptureManager;
    sk_sp<SkColorSpace> fColorSpace;
    sk_sp<SlottableResourceProvider> fSlottableProvider;
    sk_sp<SkShader> fShader;
    sk_sp<SlotManager> fSlotManager;
    std::vector<sk_sp<const skresources::ImageAsset>> fImageMap;
};

// Good: returns const sk_sp& to avoid ref-count churn when caller only borrows the factory.
SK_API const sk_sp<Factory>& GetFactory();
```

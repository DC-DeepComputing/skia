---
name: skia/core/defer-options-for-static-singletons
description: When returning function-local static singletons from lookup functions, avoid constructing options or configuration structs unconditionally outside the static initializer; defer them inside the initialization helper
rubric: safety
severity: should_fix
---

**Rule.** Do not construct configuration, options, or descriptor structs
unconditionally in lookup functions that return lazily initialized static
singletons. Defer constructing and configuring these structs inside the
dedicated static initialization helper or factory function so they only evaluate
on first initialization.

**Why.** Function-local static variables are initialized only once on first
execution. Constructing options or configuration objects unconditionally at
lookup function scope incurs redundant object creation and initialization
overhead on every subsequent retrieval call.

## Examples

Bad:

```cpp
const SkRuntimeEffect* GetKnownRuntimeEffect(StableKey stableKey) {
    // Bad: options struct is constructed on every lookup call.
    SkRuntimeEffect::Options options;
    SkRuntimeEffectPriv::SetStableKeyOnOptions(&options, static_cast<uint32_t>(stableKey));
    SkRuntimeEffectPriv::AllowPrivateAccess(&options);

    switch (stableKey) {
        case StableKey::k1DBlur4: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(4, options);
            return s1DBlurEffect;
        }
    }
}
```

Good:

```cpp
const SkRuntimeEffect* GetKnownRuntimeEffect(StableKey stableKey) {
    switch (stableKey) {
        case StableKey::k1DBlur4: {
            // Good: static initializer runs once; helper creates options lazily.
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_shader(4, stableKey);
            return s1DBlurEffect;
        }
    }
}

static SkRuntimeEffect* make_blur_1D_shader(int kernelWidth, StableKey stableKey) {
    SkRuntimeEffect::Options options = get_options(stableKey);
    return SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, ..., options);
}
```

---
name: skia/scope-local-variables-to-innermost-block
description: When scoping variables and constants, confine single-use values to innermost blocks, avoid redundant bare scope blocks, and declare shared constants as static constexpr
rubric: readability
severity: should_fix
---

**Rule.** Declare local variables, constants, and diagnostic setup in the
innermost block or conditional branch where consumed, and inline obvious
single-use constants rather than defining unnecessary named variables. Avoid
introducing bare scoping blocks (`{ ... }`) without control flow or when
artificial RAII lifetime restrictions are unnecessary before returning; return
directly once ready. Conversely, when compile-time constants are shared across
multiple helper functions or local classes in a translation unit (.cpp), declare
them at file scope as `static constexpr` rather than scoping them inside a class
and duplicating hardcoded literals.

**Why.** Confining single-use variables or constants to their innermost block
minimizes lifetime and cognitive load, while avoiding bare blocks eliminates
unnecessary nesting and indentation. Conversely, hoisting constants shared
between file-scope helpers and classes to file-scope `static constexpr` prevents
duplicating magic numbers and ensures consistent configuration across the
translation unit.

## Examples

Bad:

```cpp
// Variant 1: Hoisting single-use constant instead of inlining
T* set(T val) {
    static constexpr int kGrowFactor = 2;
    if (shouldGrow) {
        this->resize(fCapacity > 0 ? fCapacity * kGrowFactor : 4);
    }
    return this->uncheckedSet(std::move(val));
}

// Variant 2: Diagnostic setup computed outside the failing branch
if (bitErrors > 0) {
    uint32_t hexP0X;
    std::memcpy(&hexP0X, &p0.fX, 4);
    if (bitErrors > kErrorLimit) {
        ERRORF(reporter, "Fail: 0x%08x", hexP0X);
    }
}

// Variant 3: Artificial bare scoping blocks or unnecessary RAII lifetime nesting
static float get_max(const Metadata& m) {
    {
        ContentInfo info;
        if (m.getInfo(&info)) { return info.fMax; }
    }
    return 1000.f;
}
skgpu::UniqueKey MakeKey(...) {
    skgpu::UniqueKey key;
    {
        skgpu::UniqueKey::Builder builder(&key, kDomain, 1);
    }
    return key;
}

// Variant 4: Class-scoped constant forces helper to duplicate magic numbers
static SkBitmap make_panel() {
    SkBitmap p;
    p.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kOpaque_SkAlphaType));
    return p;
}
class Slide { static constexpr float kPanelSize = 100.f; };
```

Good:

```cpp
// Variant 1: Inlining obvious single-use factor in narrowest block
T* set(T val) {
    if (shouldGrow) {
        this->resize(fCapacity > 0 ? fCapacity * 2 : 4);
    }
    return this->uncheckedSet(std::move(val));
}

// Variant 2: Diagnostic setup scoped to the specific branch needing it
if (bitErrors > kErrorLimit) {
    uint32_t hexP0X;
    std::memcpy(&hexP0X, &p0.fX, 4);
    ERRORF(reporter, "Fail: 0x%08x", hexP0X);
}

// Variant 3: Declaring sequential variables at function scope and returning directly
static float get_max(const Metadata& m) {
    ContentInfo info;
    if (m.getInfo(&info)) { return info.fMax; }
    return 1000.f;
}
skgpu::UniqueKey MakeKey(...) {
    skgpu::UniqueKey key;
    skgpu::UniqueKey::Builder builder(&key, kDomain, 1);
    return key;
}

// Variant 4: Shared constant declared at file scope as static constexpr
static constexpr float kPanelSize = 100.f;
static SkBitmap make_panel() {
    SkBitmap p;
    p.setInfo(SkImageInfo::Make(kPanelSize, kPanelSize, kRGBA_8888_SkColorType, kOpaque_SkAlphaType));
    return p;
}
class Slide { /* uses kPanelSize */ };
```

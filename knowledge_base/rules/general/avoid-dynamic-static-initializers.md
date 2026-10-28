---
name: skia/avoid-dynamic-static-initializers
description: When declaring namespace- or class-scope static variables, avoid dynamic initializers and free atomics; use constexpr tables, on-demand helpers, or function-local statics
rubric: readability
severity: blocking
---

**Rule.** Do not declare namespace- or class-static variables of types requiring
runtime dynamic initialization (such as std::vector, std::string, std::map, or
types with non-trivial constructors like SkColorInfo and SkMaskGamma), nor
declare free static variables (such as std::atomic ID counters) at file or
namespace scope. Instead, use constexpr data structures (such as constexpr
std::array) for compile-time lookup tables, provide static helper functions that
construct instances on demand, or use lazily initialized function-local static
variables (returning references for immutable singletons or encapsulating
mutable counters inside accessor functions).

**Why.** Dynamic initialization of namespace- or class-static variables runs
before main() at library load time, increasing application startup latency,
risking static initialization order bugs, and breaking downstream embedding
checks (such as Chromium's check_static_initializers bot). Free mutable statics
at file or namespace scope also leak state across the translation unit. Scoping
statics locally inside accessor or factory functions defers initialization until
first call, which is thread-safe in C++11, encapsulates state, and eliminates
startup static initializers.

## Examples

Bad:

```cpp
// Static containers requiring dynamic initialization:
const std::vector<SkColorInfo> PrecompileImageShader::kDefaultColorInfos = {
        kDefaultColorInfoPremul, kDefaultColorInfoSRGB, kDefaultColorInfoGeneral,
        kDefaultColorInfoAlphaOnly};

// Class instance with non-trivial constructor causing static initializer at load time:
static SkMaskGamma gLinearMaskGamma = SkMaskGamma();

// Free static atomic counter at file/namespace scope:
static std::atomic<uint32_t> gNextUniqueID{1};
```

Good:

```cpp
// Static helper function returning instances on demand:
static std::vector<SkColorInfo> DefaultColorInfos() {
    return { DefaultColorInfoPremul(), DefaultColorInfoSRGB() };
}

// Function-local static lazily initialized on first use:
static const SkMaskGamma& linear_gamma() {
    static const SkMaskGamma kLinear;
    return kLinear;
}

// Scoped function-local static atomic counter:
uint32_t next_unique_id() {
    static std::atomic<uint32_t> gNextUniqueID{SK_InvalidUniqueID + 1};
    return gNextUniqueID.fetch_add(1, std::memory_order_relaxed);
}
```

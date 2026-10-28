---
name: skia/codec/decouple-alternative-backend-entrypoints
description: When introducing alternative backend implementations in codecs, expose a canonical dispatching entrypoint, designate backend functions for tests, and isolate them into dedicated files
rubric: design
severity: should_fix
---

**Rule.** When introducing an alternative or migrating backend implementation
(such as Rust vs. C++ for ICC profile parsing or decoding), expose a single
canonical entrypoint for production consumers that dispatches internally based
on configuration. Isolate backend-specific or language-specific (such as Rust
FFI) declarations and implementations into dedicated header and source files
(e.g., *Rust.h / *Rust.cpp), befriending them in core classes if necessary, and
document them as test-only rather than embedding them directly in shared headers
under #ifdef guards.

**Why.** Exposing backend-specific entrypoints alongside generic ones tempts
consumers to invoke specific backends directly, causing widespread code churn
across codecs during rollouts. Embedding optional or Rust FFI declarations
directly in shared headers pollutes core interfaces with preprocessor guards and
complicates build configurations; isolating them into dedicated files keeps core
headers clean and decouples optional target compilation.

## Examples

Bad:

```cpp
// In src/codec/SkCodecPriv.h
// Bad: Exposes backend-specific entrypoint in core header, tempting callers
// and cluttering shared interfaces with optional preprocessor guards.
class ColorProfile {
public:
    static std::unique_ptr<ColorProfile> MakeICCProfile(sk_sp<const SkData>);
#if defined(SK_CODEC_COLOR_PROFILE_PARSE_WITH_RUST)
    static std::unique_ptr<ColorProfile> MakeICCProfileWithRust(sk_sp<const SkData>);
#endif
};
```

Good:

```cpp
// In src/codec/SkCodecPriv.h
// Good: Canonical entrypoint dispatches internally; backend-specific test entrypoint
// is isolated in a dedicated header/source and befriended in the core class.
class ColorProfile {
public:
    static std::unique_ptr<ColorProfile> MakeICCProfile(sk_sp<const SkData>);
    static std::unique_ptr<ColorProfile> MakeICCProfileWithSkCMS(sk_sp<const SkData>);
#if defined(SK_CODEC_COLOR_PROFILE_PARSE_WITH_RUST)
    friend std::unique_ptr<ColorProfile> MakeICCProfileWithRust(sk_sp<const SkData>);
#endif
};

// In src/codec/SkCodecColorProfileRust.h
namespace SkCodecs {
// For testing only; production code calls MakeICCProfile().
std::unique_ptr<ColorProfile> MakeICCProfileWithRust(sk_sp<const SkData> data);
}
```

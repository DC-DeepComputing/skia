---
name: skia/guard-conditionally-compiled-symbols
description: When referencing conditionally compiled symbols, guard usages under positive feature flags, but avoid guarding abstract factories that handle backends internally
rubric: design
severity: blocking
---

**Rule.** When referencing symbols, types, module switch cases, or concrete
modular backend implementations (such as image codecs under SK_CODEC_DECODES_*,
GPU modules under SK_GRAPHITE or SK_GANESH, or concrete text factories like
SkUnicode::MakeClientBasedUnicode under SK_UNICODE_CLIENT_IMPLEMENTATION), guard
usages with matching positive preprocessor flags; do not leave them unguarded or
use negative guards (#ifndef). Conversely, do not guard calls to abstract or
default factory functions (such as SkUnicode::Make()) with backend-specific
preprocessor defines; let the factory resolve backends internally and check the
returned pointer for nullptr.

**Why.** Omitting positive guards around conditionally compiled symbols causes
linker errors or false assumptions about build defaults, while negative guards
(#ifndef) break configurations where multiple backends coexist or are disabled.
Conversely, guarding abstract factory calls with concrete backend defines
couples consumers to a specific backend, disables functionality when alternative
implementations (e.g., ICU4X vs. ICU) are configured, and leaks internal backend
flags into public consumer modules.

## Examples

Bad:

```cpp
// Bad: Leaving optional codecs or backend modules unguarded
#if defined(SK_GRAPHITE)
    case ModuleType::kGraphiteFrag: return M(sksl_graphite_frag);
#endif
    case ModuleType::kGraphiteFragES2: return M(sksl_graphite_frag_es2);

// Bad: Using negative guard of one backend to enable an alternative backend
#ifndef SK_UNICODE_ICU_IMPLEMENTATION
    fUnicode = SkUnicode::MakeClientBasedUnicode(...);
#endif

// Bad: Guarding abstract factory calls with a concrete backend define
#ifdef SK_UNICODE_ICU_IMPLEMENTATION
if (auto skuni = SkUnicode::Make()) {
    *fText.writable() = skuni->toUpper(*fText);
}
#endif
```

Good:

```cpp
// Good: Every optional module and codec explicitly guarded with positive flags
#if defined(SK_GRAPHITE)
    case ModuleType::kGraphiteFrag: return M(sksl_graphite_frag);
    case ModuleType::kGraphiteFragES2: return M(sksl_graphite_frag_es2);
#endif

#if !defined(SK_UNICODE_ICU_IMPLEMENTATION) && defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
    fUnicode = SkUnicode::MakeClientBasedUnicode(...);
#endif

// Good: Abstract factory called unconditionally and checked for nullptr
if (auto skuni = SkUnicode::Make()) {
    *fText.writable() = skuni->toUpper(*fText);
}
```

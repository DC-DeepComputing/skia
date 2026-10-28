---
name: skia/prefix-header-macros-with-sk
description: When defining non-file-scoped or header macros, prefix them with SK_ (or GR_), and use structured subsystem patterns like SK_<SUBSYSTEM>_IS_<VARIANT>
rubric: readability
severity: should_fix
---

**Rule.** Prefix macros that have greater than file scope (such as those defined
in header files) with SK_ (or GR_ in Ganesh) followed by the subsystem
identifier (e.g., SK_<SUBSYSTEM>_<NAME>). For mutually exclusive configuration
or mode flags, use consistent structured naming like SK_<SUBSYSTEM>_IS_<VARIANT>
rather than ad-hoc suffixes or inverted prefixes.

**Why.** Preprocessor macros lack namespace isolation and leak across include
boundaries. Prefixing non-file-scoped macros with SK_ (or GR_) avoids global
symbol collisions, while consistent subsystem prefixes and structured variant
naming patterns clarify ownership and streamline configuration gating.

## Examples

Bad:

```cpp
// Bad: Lacks SK_ prefix and uses an ad-hoc suffix with inverted ordering for a mode toggle.
#ifdef RASTER_PIPELINE_BLITTER_TRACE_ON
SkBlitterTrace gSkRPBlitterTrace("RP", false);
...
#endif
```

Good:

```cpp
// Good: Uses SK_ prefix, subsystem identifier, and structured IS_<VARIANT> pattern.
#ifdef SK_BLITTER_TRACE_IS_RASTER_PIPELINE
SkBlitterTrace gSkRPBlitterTrace("RP", false);
...
#endif
```

---
name: skia/document-unfamiliar-build-properties-in-commit-message
description: When introducing uncommon, platform-specific, or unfamiliar build target properties or dependency types (like Soong whole_static_libs), provide reference documentation links and explanations in the commit message
rubric: readability
severity: should_fix
---

**Rule.** When introducing uncommon, platform-specific, or unfamiliar build
target properties, dependency attributes, or linker rules (such as Android
Soong's `whole_static_libs`), provide reference documentation links and
explanatory context in the commit message description. Explain why the specific
property is necessary and how it differs from standard dependencies to aid
reviewers and future maintainers diagnosing build regressions.

**Why.** Platform-specific build systems and specialized target properties are
unfamiliar to most maintainers and can alter linking behavior in subtle ways.
Documenting official reference links and technical rationale in the commit
message provides essential context for code review and future build
troubleshooting.

## Examples

Bad:

```cpp
Reason for revert: Re-landing with fix (include libmediandk as a direct dependency in skia_defaults and include libcrabbyavif_ffi as a whole_static_libs dependency).
```

Good:

```cpp
Reason for revert: Re-landing with fix (include libmediandk as a direct dependency in skia_defaults and include libcrabbyavif_ffi as a whole_static_libs dependency - documented in go/Android.bp and https://ci.android.com/builds/submitted/12477084/linux/latest/cc.html#cc_library.whole_static_libs).
```

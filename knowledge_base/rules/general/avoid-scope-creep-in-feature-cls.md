---
name: skia/avoid-scope-creep-in-feature-cls
description: When authoring focused feature, bugfix, or refactoring CLs, omit unrelated drive-by edits, behavioral tweaks, branch leakage, and orphaned preparatory renames or refactorings
rubric: change_scope
severity: should_fix
---

**Rule.** Do not combine large-scale refactorings, broad deletions, drive-by
fixes, behavioral tweaks, untouched line reformatting, branch leakage, scratch
files, test deletions, or unused utilities with focused updates or bug fixes. In
particular, isolate behavioral tweaks or bug fixes producing test/Gold diffs
into dedicated CLs. When a motivating feature or API is dropped during review,
revert any preparatory file renames, refactorings, or header churn introduced
for it. Verify diffs are strictly scoped to modified logic, rebases retain
commits and flags, and added files are intentional and integrated into build
targets. Keep changes scoped to their primary objective, deferring secondary
cleanups to follow-up CLs.

**Why.** Bundling major refactorings, broad deletions, branch leakage,
unreferenced scratch files, drive-by formatting on untouched lines, or unrelated
bug fixes into focused changes inflates patch size, pollutes git blame history,
obscures primary intent, and complicates bisection. Retaining preparatory
renames or refactorings after their motivating feature is dropped leaves
orphaned churn across unrelated files. Isolating cleanups, build adjustments,
test removals, and distinct fixes into dedicated CLs minimizes review surface
and prevents regressions.

## Examples

Bad:

```cpp
// 1. Unused utility accessor edits bundled into a focused feature or refactoring CL:
Iter begin() const { return Iter(fHead); }
Iter end() const { return Iter(); }

// 2. Bundling a sweeping API refactoring (~1000 lines) into a focused feature/bugfix CL:
void VulkanPreferredFeatures::addFeaturesToEnable(std::vector<const char*>& appExtensions) { ... }

// 3. Bundling drive-by bug fixes / Gold diff changes into a refactoring CL:
// In a CL refactoring atlas padding:
float3 devCorner = matrix * corner.xy1;
stepLocalCoords = corner;
// Unrelated drive-by depth adjustment addressing a separate z-fighting bug:
return float4(devCorner.xy, depth*devCorner.z, devCorner.z);

// 4. Bundling unrelated unit test deletions or cleanups into a feature CL:
-UNIX_ONLY_TEST(SkParagraph_ArabicWithLetterSpacingWhitespacesAsChrome, reporter) { ... }

// 5. Unrelated build flag deletion or accidental commit reversion during rebase:
cflags_cc += [ "-Wno-c++98-compat" ] # -Wc++98-compat-extra-semi dropped during rebase

// 6. Accidental deletion of instrumentation or WIP edits leaked from another branch:
- TRACE_EVENT_INSTANT0("skia.gpu.cache", TRACE_FUNC, TRACE_EVENT_SCOPE_THREAD);

// 7. Drive-by reformatting of untouched lines or stray unreferenced scratch files:
// modules/skparagraph/slides/test.cpp (unreferenced in headers and build files)

// 8. Retaining preparatory file renames or refactorings after the motivating feature is dropped:
// src/ports/SkTypeface_FreeType.h renamed to SkTypeface_FreeType_priv.h across 20 files,
// but the planned public header include/ports/SkTypeface_FreeType.h was abandoned during review
```

Good:

```cpp
// 1. Leave shared utility classes untouched when edits are unused by the core change:
Iter begin() { return Iter(fHead); }

// 2. Keep focused feature CLs scoped; defer broad parameter/cleanup pivots to follow-up:
void VulkanPreferredFeatures::addFeaturesToEnable(std::vector<const char*>& appExtensions, VkPhysicalDeviceFeatures2& appFeatures) { ... }

// 3. Keep refactoring CLs behavior-neutral; extract fixes causing Gold diffs to a separate CL:
float3 devCorner = matrix * corner.xy1;
stepLocalCoords = corner;
return float4(devCorner.xy, depth, devCorner.z);

// 4. Retain unrelated unit tests; split intentional test removals into a dedicated cleanup CL:
UNIX_ONLY_TEST(SkParagraph_ArabicWithLetterSpacingWhitespacesAsChrome, reporter) { ... }

// 5. Keep build configurations untouched; verify rebases retain recently landed flags:
cflags_cc += [ "-Wno-c++98-compat", "-Wc++98-compat-extra-semi" ]

// 6. Retain existing instrumentation; keep CL strictly scoped to its stated purpose:
TRACE_EVENT_INSTANT0("skia.gpu.cache", TRACE_FUNC, TRACE_EVENT_SCOPE_THREAD);

// 7. Retain existing formatting on untouched lines and omit temporary scratch files

// 8. Revert preparatory file renames and keep original filenames when the motivating API is dropped
```

---
name: skia/infra/retain-oldest-supported-os-ci-coverage
description: When upgrading or retiring CI bot configurations in infra/bots, retain test coverage for critical platform dependencies like NativeFonts on the oldest supported OS version
rubric: testing
severity: should_fix
---

**Rule.** When upgrading, migrating, or retiring CI test bot configurations in
infra/bots (such as jobs.json or task generator scripts), retain test coverage
for critical platform configurations (such as NativeFonts, FreeType, FontConfig,
and GPU drivers) on the project's oldest supported OS or platform release. Do
not decommission older OS test bot definitions without ensuring baseline
platform compatibility remains validated.

**Why.** Skia supports older OS baselines matching downstream embedder
requirements (such as Chromium on Debian 10 or Ubuntu 18.04). Upgrading test
bots exclusively to newer OS releases risks missing regressions where Skia code
inadvertently relies on newer system font engines, libraries, or GPU driver
features not available on older supported platforms.

## Examples

Bad:

```cpp
// Retiring the oldest OS bots without preserving critical native library/font tests:
-  {"name": "Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All-NativeFonts"},
+  {"name": "Test-Ubuntu24.04-Clang-GCE-CPU-AVX2-x86_64-Debug-All-NativeFonts"},
```

Good:

```cpp
// Preserving NativeFonts test coverage on the oldest supported OS:
+  {"name": "Test-Ubuntu18-Clang-Golo-CPU-AVX2-x86_64-Debug-All-NativeFonts"},
   {"name": "Test-Ubuntu24.04-Clang-GCE-CPU-AVX2-x86_64-Debug-All-NativeFonts"},
```

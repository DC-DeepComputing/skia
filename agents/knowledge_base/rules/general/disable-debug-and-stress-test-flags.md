---
name: skia/disable-debug-and-stress-test-flags
description: When adding or modifying local diagnostic flags, verbose logging switches, or stress-test toggles, ensure they remain disabled by default before submitting
rubric: readability
severity: should_fix
---

**Rule.** Ensure that temporary debug flags, verbose logging switches, and
expensive stress-test or benchmark toggles (such as constexpr bool k... = false
or static const bool) remain disabled (false) by default before submitting
changes. Do not leave local debugging overrides, random trial loops, or
diagnostic dumps enabled in submitted code.

**Why.** Diagnostic flags, verbose logging switches, and stress-test toggles are
intended for local troubleshooting and manual verification. Leaving them enabled
in submitted commits bloats continuous integration (CI) test runtimes, slows
presubmit bots, generates disk-cluttering dumps, and pollutes test logs with
noisy output.

## Examples

Bad:

```cpp
// Bad: Stress-test flag left enabled, forcing expensive random trials in CI
constexpr bool kRunRandomTest = true;

DEF_TEST(MFC_myers_brute_force_random_comparison, r) {
    if constexpr (!kRunRandomTest) {
        return;
    }
    run_expensive_random_trials(r);
}
```

Good:

```cpp
// Good: Stress-test toggle disabled by default so automated CI remains fast
constexpr bool kRunRandomTest = false;

DEF_TEST(MFC_myers_brute_force_random_comparison, r) {
    if constexpr (!kRunRandomTest) {
        return;
    }
    run_expensive_random_trials(r);
}
```

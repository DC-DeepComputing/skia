---
name: skia/annotate-todos-with-bug-and-description
description: When writing, refactoring, or narrowing code with TODO comments and workarounds, format deferred work with bug URLs, update remaining platform requirements, and preserve existing TODOs
rubric: readability
severity: should_fix
---

**Rule.** Remove transient scratchpad notes (such as // KEEP or // CHECK THIS!)
and obsolete TODO/FIXME comments once addressed. When deferring work,
documenting planned enhancements, or implementing temporary workarounds, format
notes as actionable TODOs with a complete, clickable issue tracker URL or ID
(e.g., TODO(https://crbug.com/123): description or TODO(b/123):
description)—never leaving untracked notes, domain-less shorthands, bare
usernames, or incomplete placeholders. When refactoring or mirroring downstream
code, preserve existing and workaround TODOs while workarounds remain; when
narrowing fallback or emulation logic, update accompanying TODOs to explicitly
specify which remaining platforms or configurations still require the
workaround.

**Why.** Attaching a full URL or valid issue ID (such as
TODO(https://crbug.com/123): or TODO(b/123):) ensures deferred work is clickable
and actionable. Removing obsolete annotations, preserving active workarounds,
and updating workaround descriptions when narrowing fallbacks prevents
misleading maintainers about which remaining platforms or configurations still
block code removal.

## Examples

Bad:

```cpp
// Stale TODO, incomplete placeholder, domain-less shorthand, or scratchpad note
bool fHasGradientBuffer = false;  // KEEP
// TODO(crbug/1339019): Implement this if needed.
// TODO(michaelludwig, b/): TextureUploadWriter isn't very useful

// Dropped existing TODO during refactoring
BlendFormula formula = coverage == Coverage::kLCD ? GetLCDBlendFormula() : GetBlendFormula();

// Outdated workaround TODO omits remaining platforms when narrowing fallback
// TODO(b/399640773): Remove this once Dawn natively supports partial resolve.
#if defined(__EMSCRIPTEN__)
    emulateResolve = true;
#endif
```

Good:

```cpp
// Actionable TODO with clickable bug URL and tracked issue
bool fHasGradientBuffer = false;
// TODO(https://crbug.com/1339019): Implement this if needed.
// TODO(b/390473370): TextureUploadWriter isn't very useful

// Preserved existing TODO across refactoring
// TODO(b/123): Determine whether draw is opaque and pass that to GetBlendFormula.
BlendFormula formula = coverage == Coverage::kLCD ? GetLCDBlendFormula() : GetBlendFormula();

// Workaround TODO specifies remaining platforms blocking removal
// TODO(b/399640773): Emulate on WASM/WebGPU which lack partial resolve.
#if defined(__EMSCRIPTEN__)
    emulateResolve = true;
#endif
```

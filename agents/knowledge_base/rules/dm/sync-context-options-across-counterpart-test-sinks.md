---
name: skia/dm/sync-context-options-across-counterpart-test-sinks
description: When updating context or test option setup in DM test sinks, update all counterpart sink implementations sharing parallel initialization consistently
rubric: correctness
severity: should_fix
---

**Rule.** When updating or refactoring context configuration, test options, or
private option structures (such as TestOptions or ContextOptionsPriv) in a DM
test sink, verify and update all counterpart sink implementations sharing
parallel setup logic (such as GraphiteSink and GraphitePrecompileTestingSink).
Do not modify option lifecycle, assertions, or overrides in only one sink while
leaving sibling sinks with outdated configuration.

**Why.** Test harness sinks frequently duplicate context initialization, option
overrides, and validation assertions across sibling classes to evaluate
different rendering or pipeline caching modes under identical conditions.
Updating option management in only a subset of sinks introduces configuration
drift, inconsistent test environments, or assertion failures across counterpart
test configurations.

## Examples

Bad:

```cpp
// In GraphitePrecompileTestingSink::draw, manual ContextOptionsPriv setup was removed,
// but counterpart GraphiteSink::draw retained outdated manual setup:
Result GraphiteSink::draw(const Src& src, SkBitmap* dst, SkWStream* dstStream, SkString* log) const {
    skiatest::graphite::TestOptions options = fOptions;
    SkASSERT(!options.fContextOptions.fOptionsPriv);
    skgpu::graphite::ContextOptionsPriv optionsPriv;
    options.fContextOptions.fOptionsPriv = &optionsPriv;
    ...
}
```

Good:

```cpp
// Sibling test sinks are updated consistently when TestOptions manages ContextOptionsPriv:
Result GraphiteSink::draw(const Src& src, SkBitmap* dst, SkWStream* dstStream, SkString* log) const {
    skiatest::graphite::TestOptions options = fOptions;
    SkASSERT(!options.fContextOptions.fOptionsPriv);
    ...
}
```

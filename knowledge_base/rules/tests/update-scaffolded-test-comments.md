---
name: skia/tests/update-scaffolded-test-comments
description: When authoring, scaffolding, or updating tests or GMs, document the purpose and expected outcome for non-obvious tests, and keep introductory descriptions and calculation breakdowns accurate
rubric: readability
severity: should_fix
---

**Rule.** When authoring, scaffolding, or updating test cases and GMs (such as
DEF_TEST, DEF_GM, or JS gm()), provide an explanatory comment documenting the
purpose and expected outcome when the test's intent or visual output is not
immediately obvious from the name alone. Ensure introductory comments accurately
describe the component and behavior being verified rather than leaving stale or
copy-pasted boilerplate. Additionally, when modifying test parameters or
assertion targets, keep accompanying comments explaining expected values or
arithmetic breakdowns synchronized with actual assertion expressions.

**Why.** Without clear introductory documentation, reviewers, gardeners, and
future maintainers cannot easily discern whether a test's setup and visual
output or assertions are intended or broken. Stale, copy-pasted, or out-of-sync
comments contradict test logic, obscure test intent during triage, and create
false confidence by claiming unasserted test coverage or presenting arithmetic
breakdowns that contradict assertion expressions.

## Examples

Bad:

```cpp
// Variant 1: Non-obvious test or GM with no explanatory comment
gm('paragraph_client_icu', (canvas) => {
    const fontMgr = CanvasKit.FontMgr.FromData(robotoFontBuffer);
    ...
});

// Variant 2: Stale comment copied from template file describing an unrelated subsystem
// These are intended to be unit tests of the PaintParamsKeyBuilder and PaintParamsKey.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(InnerFillTest, reporter, context,
                                   CtsEnforcement::kNextRelease) { ... }

// Variant 3: Stale arithmetic breakdown contradicting updated assertion value
// The 144 comes from:
//     4 gradient flavors * 3 types * 4 threads
REPORTER_ASSERT(reporter, stats.fGraphicsCacheHits + stats.fGraphicsCacheMisses == 48);
```

Good:

```cpp
// Variant 1: Documented test purpose and expected outcome for non-obvious test
// Verify that Client ICU produces paragraph layout results matching normal ICU for English text.
gm('paragraph_client_icu', (canvas) => {
    const fontMgr = CanvasKit.FontMgr.FromData(robotoFontBuffer);
    ...
});

// Variant 2: Synchronized comment accurately describing the test purpose and behavior
// These are unit tests that ensure the secondary "inner fill" draw is recorded when possible.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(InnerFillTest, reporter, context,
                                   CtsEnforcement::kNextRelease) { ... }

// Variant 3: Arithmetic breakdown kept in sync with updated assertion expression
// The 48 comes from:
//     4 gradient flavors * 3 types * 4 threads
REPORTER_ASSERT(reporter, stats.fGraphicsCacheHits + stats.fGraphicsCacheMisses == 48);
```

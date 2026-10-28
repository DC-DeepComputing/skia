---
name: skia/infra/document-ci-test-skips-with-tracking-bug
description: When suppressing tests in CI bot configuration files like dm_flags.go, document skips with tracking bugs, narrow conditions, and preserve alphabetical ordering
rubric: readability
severity: nit
---

**Rule.** When suppressing tests in CI bot configuration (such as skip() in
infra/bots/gen_tasks_logic/dm_flags.go), document each suppression with an
inline comment citing a tracking issue (e.g., '// b/<id> - <reason>') explaining
why the test is disabled, and narrow the suppression condition as tightly as
possible to the specific failing backend, OS, hardware model, or build
configuration rather than skipping tests globally. Additionally, maintain strict
alphabetical order by test or GM name when inserting entries into sorted skip
blocks.

**Why.** Skipping tests without documenting an issue reference creates
unmonitored technical debt that is easily forgotten, while broad suppressions
risk masking unrelated regressions. In sorted blocks of skip declarations,
maintaining alphabetical order makes entries easy to locate, prevents duplicate
suppressions, and minimizes merge conflicts across concurrent changes.

## Examples

Bad:

```cpp
// Bad: missing tracking bug, overly broad suppression, and out of alphabetical order
if b.ExtraConfig("Metal") {
	skip(ALL, "test", ALL, "ShaderTestNestedBlendsGraphite")
	skip(ALL, "test", ALL, "skbug6653")
	skip(ALL, "test", ALL, "SimplifyPaintTest")
	skip(ALL, "test", ALL, "ThreadedPipelineCompilePurgingTest")
}
```

Good:

```cpp
// Good: narrowed condition, tracking bug citation, and alphabetical order preserved
if b.ExtraConfig("Metal") {
	if b.ExtraConfig("TSAN") {
		skip(ALL, "test", ALL, "ShaderTestNestedBlendsGraphite")
		skip(ALL, "test", ALL, "SimplifyPaintTest")
		skip(ALL, "test", ALL, "skbug6653")
		// b/389706939 - Dawn/Metal reports a data race for LazyClearCountForTesting w/ TSAN
		skip(ALL, "test", ALL, "ThreadedPipelineCompilePurgingTest")
	}
}
```

---
name: skia/document-temporary-assertions-with-todo
description: When adding assertions for temporary limitations or planned transformations, document them with an attributed TODO(bug) rather than ambiguous expectation comments
rubric: readability
severity: nit
---

**Rule.** When adding or modifying assertions (such as SkASSERT) that enforce
temporary work-in-progress limitations, planned compiler transforms, or
incomplete feature support rather than permanent architectural invariants,
document them with an explanatory TODO(b/...) or TODO(bug) comment referencing
an active tracking issue. State the specific prerequisite condition, transform,
or capability required before the assertion can be removed. Avoid ambiguous
phrasing like 'we expect X' that leaves readers uncertain whether an assumption
is already guaranteed by an earlier pipeline stage or represents deferred work;
if an assumption is an established upstream invariant, document where and how it
is enforced instead.

**Why.** Assertions guarding incomplete features, deferred passes, or planned
transformations look identical to permanent architectural invariants unless
explicitly documented. Vague comments like 'we expect X' blur the line between
upstream guarantees and unhandled future work, whereas an attributed TODO
preserves design intent, tracks technical debt, and clarifies the concrete
criteria for removing or retaining the assertion.

## Examples

Bad:

```cpp
// Bad: Undocumented assertion enforcing a temporary limitation looks like a permanent invariant
if (bindDstAsInputAttachment) {
    SkASSERT(fTargetTexture && fTargetTexture->numSamples() == 1);
    this->updateAndBindInputAttachment(*fTargetTexture,
                                       VulkanGraphicsPipeline::kDstAsInputDescSetIndex,
                                       fResourceProvider->mockPipelineLayout());
}

// Bad: Ambiguous "we expect" comment obscures whether the transform is deferred work or an existing guarantee
// We expect the IR to already have been transformed into this form before the codegen stage.
Operator op = p.getOperator();
SkASSERT(op.kind() != Operator::Kind::PLUSPLUS);
SkASSERT(op.kind() != Operator::Kind::MINUSMINUS);
```

Good:

```cpp
// Good: Attributed TODO explains the limitation and specifies the removal condition
if (bindDstAsInputAttachment) {
    // TODO(b/390458117): This assert can be removed once the sample loading shader supports
    // sample counts > 1.
    SkASSERT(fTargetTexture && fTargetTexture->numSamples() == 1);
    this->updateAndBindInputAttachment(*fTargetTexture,
                                       VulkanGraphicsPipeline::kDstAsInputDescSetIndex,
                                       fResourceProvider->mockPipelineLayout());
}

// Good: Explicit TODO(bug) clarifies that assertions guard planned transforms
// TODO(skia:14082): The following assertions may be hit until the proposed transforms have
// been implemented. The IR should be transformed such that prefix increment/decrement
// expressions never appear in the IR.
Operator op = p.getOperator();
SkASSERT(op.kind() != Operator::Kind::PLUSPLUS);
SkASSERT(op.kind() != Operator::Kind::MINUSMINUS);
```

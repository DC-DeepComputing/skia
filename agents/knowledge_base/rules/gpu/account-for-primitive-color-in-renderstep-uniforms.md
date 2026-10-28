---
name: skia/gpu/account-for-primitive-color-in-renderstep-uniforms
description: When determining whether a Graphite RenderStep requires fragment shader uniforms, verify whether the step emits primitive color in addition to checking coverage
rubric: correctness
severity: should_fix
---

**Rule.** When determining whether a Graphite RenderStep references uniforms in
fragment shader code (such as in RenderStep::usesUniformsInFragmentSkSL or
related fragment omission checks), verify whether the step emits primitive color
via emitsPrimitiveColor() in addition to checking whether it computes coverage.
Do not rely solely on coverage() != Coverage::kNone.

**Why.** In Graphite, a RenderStep can contribute to the fragment stage either
by calculating coverage (fragmentCoverageSkSL) or by generating primitive color
(fragmentColorSkSL). Omitting the emitsPrimitiveColor() check leads to fragment
shader stages dropping uniform declarations or bindings required by steps that
output primitive color.

## Examples

Bad:

```cpp
virtual bool usesUniformsInFragmentSkSL() const {
    return this->coverage() != Coverage::kNone;
}
```

Good:

```cpp
virtual bool usesUniformsInFragmentSkSL() const {
    return this->coverage() != Coverage::kNone || this->emitsPrimitiveColor();
}
```

---
name: skia/avoid-ableist-language-dummy
description: When writing comments, identifiers, or parameter annotations, avoid the term 'dummy'; prefer precise alternatives like 'mock', 'stub', 'placeholder', 'unused', or parameter labels
rubric: readability
severity: nit
---

**Rule.** Do not use non-inclusive or vague terminology such as 'dummy' in code
comments, parameter annotations, or identifier names. Prefer precise,
descriptive alternatives such as 'mock', 'stub', 'fake', 'placeholder',
'unused', 'ignored', or explicit inline parameter name labels (e.g.,
'/*paramName=*/{}' or '/*paramName=*/nullptr').

**Why.** Terms like 'dummy' are non-inclusive and obscure technical intent.
Using precise descriptors—such as 'mock', 'stub', or 'placeholder' for simulated
objects, and 'unused' or explicit labels for ignored parameters—clarifies
functionality, keeps the codebase welcoming, and improves maintainability.

## Examples

Bad:

```cpp
// Bad: using 'dummy' in comments, annotations, or simulated object identifiers
/*dummy value, not used*/ MTLVertexStepFunctionPerVertex,
this->pushConstants(pushConstantInfo, fResourceProvider->dummyPushConstantPipelineLayout());
int dummyVar = 0;
```

Good:

```cpp
// Good: using precise descriptors like mock, unused, or explicit parameter labels
/*appendStepFunc=*/{},
this->pushConstants(pushConstantInfo, fResourceProvider->mockPushConstantPipelineLayout());
int unusedVar = 0;
```

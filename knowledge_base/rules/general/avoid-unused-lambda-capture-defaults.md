---
name: skia/avoid-unused-lambda-capture-defaults
description: When declaring lambdas that do not reference enclosing variables, use an empty capture list [] instead of default captures [&] or [=]
rubric: readability
severity: nit
---

**Rule.** Do not specify capture defaults (such as `[&]` or `[=]`) on lambdas
that do not access or capture any variables from the enclosing scope. Use an
explicit empty capture list `[]` to clearly communicate that the lambda is
stateless and self-contained.

**Why.** Specifying default captures like `[&]` or `[=]` on stateless lambdas is
misleading to readers and risks unintentionally capturing enclosing variables or
`this`. Using an empty capture list `[]` ensures the closure is self-contained
and allows implicit conversion to function pointers where applicable.

## Examples

Bad:

```cpp
const bool validPipelines = info.fRecording->priv().taskList()->visitPipelines(
        [&](const GraphicsPipeline* pipeline) {
            return !pipeline->didAsyncCompilationFail();
        });
```

Good:

```cpp
const bool validPipelines = info.fRecording->priv().taskList()->visitPipelines(
        [](const GraphicsPipeline* pipeline) {
            return !pipeline->didAsyncCompilationFail();
        });
```

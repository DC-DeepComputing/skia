---
name: skia/prefer-captureless-lambdas-with-return-values
description: When defining local helper lambdas, prefer captureless lambdas that return status or values over capturing outer variables by reference to mutate state
rubric: readability
severity: nit
---

**Rule.** When defining local helper lambdas within a function, prefer writing
them to be captureless (`[]`) and returning a status boolean or computed value
rather than capturing enclosing variables by reference (`[&]`) to mutate outer
state. Pass required inputs and mutable arguments explicitly through the
lambda's parameter list, and update caller state from the returned result.

**Why.** Captureless lambdas make data flow, inputs, and mutations explicit at
both the definition and invocation sites. Capturing outer variables by reference
to perform side-effect mutations obscures control flow, hides dependencies, and
prevents easily extracting or reusing the helper.

## Examples

Bad:

```cpp
bool rebindDrawBuffers = false;
auto bind = [&](const BindBufferInfo& buffer,
                uint32_t stride,
                BindBufferInfo& boundBuffer,
                uint32_t& pendingBase) {
    if (buffer.fBuffer) {
        ...
        rebindDrawBuffers |= (boundBuffer != newBinding);
        boundBuffer = newBinding;
    }
};
bind(fVertices, fVertexStride, fBoundVertices, pendingBaseVertex);
```

Good:

```cpp
auto bind = [](const BindBufferInfo& buffer,
               uint32_t stride,
               BindBufferInfo& boundBuffer,
               uint32_t& pendingBase) -> bool {
    bool shouldBind = false;
    if (buffer.fBuffer) {
        ...
        shouldBind = (boundBuffer != newBinding);
        boundBuffer = newBinding;
    }
    return shouldBind;
};
if (bind(fVertices, fVertexStride, fBoundVertices, pendingBaseVertex)) {
    ...
}
```

---
name: skia/order-inputs-before-mutable-parameters
description: When declaring parameter lists, order inputs first, followed by mutable output parameters, and place callback or lambda parameters last
rubric: readability
severity: nit
---

**Rule.** Place input parameters before mutable (output or in-out) parameters in
function, method, and lambda declarations. When a function accepts a callback or
lambda parameter (such as std::function, a callable template, or a function
pointer), place the callable at the end of the parameter list—even after output
parameters—so multi-line lambdas at call sites do not bury subsequent arguments.
When adding new input parameters to an existing signature, insert them before
existing output parameters rather than appending them after outputs.

**Why.** Ordering inputs first followed by mutable parameters creates consistent
and predictable call sites where data flow proceeds from source inputs to
modified destinations. Placing callback and lambda arguments at the very end
ensures that multi-line lambdas passed at call sites do not visually obscure or
bury subsequent arguments.

## Examples

Bad:

```cpp
// Bad: input parameter (stride) placed after mutable parameters
auto bind = [&](const BindBufferInfo& buffer,
                BindBufferInfo& boundBuffer,
                uint32_t& pendingBase,
                uint32_t stride) { ... };

// Bad: callback placed before output pointer, burying info at call sites
void FindFunctionsToSpecialize(const Program& program,
                               const ParameterMatchesFn& specializationFn,
                               SpecializationInfo* info);
// Call site:
FindFunctionsToSpecialize(program, [](const Variable& v) {
    return v.type().isSampler();
}, &info);
```

Good:

```cpp
// Good: inputs first (buffer, stride), followed by mutable parameters
auto bind = [&](const BindBufferInfo& buffer,
                uint32_t stride,
                BindBufferInfo& boundBuffer,
                uint32_t& pendingBase) { ... };

// Good: callback placed at end of parameter list after output pointer
void FindFunctionsToSpecialize(const Program& program,
                               SpecializationInfo* info,
                               const ParameterMatchesFn& specializationFn);
// Call site:
FindFunctionsToSpecialize(program, &info, [](const Variable& v) {
    return v.type().isSampler();
});
```

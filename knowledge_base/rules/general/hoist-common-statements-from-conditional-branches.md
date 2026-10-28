---
name: skia/hoist-common-statements-from-conditional-branches
description: When conditional branches duplicate identical statements, assertions, function calls, aggregate returns, or boilerplate strings, hoist common logic or isolate differing arguments using ternaries
rubric: design
severity: should_fix
---

**Rule.** When conditional branches duplicate identical leading assertions,
argument evaluations, initialization statements, function calls, or trailing
validation on a computed variable, hoist the shared logic outside the
conditional blocks (declaring conditionally assigned variables beforehand).
Similarly, when branches differ by only one or two arguments, struct fields, or
formatted keywords, isolate the differing values using inline ternaries or local
variables and consolidate to a single call site, return statement, or template
instead of duplicating entire invocations across branches.

**Why.** Duplicating identical assertions, expressions, function calls, trailing
validation, aggregate initializations, or boilerplate declaration strings across
branches bloats code, obscures what actually differs between branches, and risks
maintenance divergence when updates miss a branch. Hoisting common statements
before or after branching flattens control flow and makes branch variations
immediately obvious.

## Examples

Bad:

```cpp
// Bad: Duplicating leading assertions and common argument evaluations across branches
if (arg2.type().componentType().isFloat()) {
    SkASSERT(arg0.type().matches(arg1.type()));
    if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
        return unsupported();
    }
    // ...
}
if (arg2.type().componentType().isBoolean()) {
    SkASSERT(arg0.type().matches(arg1.type()));
    if (!this->pushExpression(arg0) || !this->pushExpression(arg1) || !this->pushExpression(arg2)) {
        return unsupported();
    }
    // ...
}

// Bad: Duplicating identical function call across branches for one differing argument
sk_sp<ImageAsset> asset;
if (slotID) {
    asset = fResourceProvider->loadImageAsset(path->begin(), name->begin(), slotID->begin());
} else {
    asset = fResourceProvider->loadImageAsset(path->begin(), name->begin(), id->begin());
}

// Bad: Duplicating factory return across branches for optional pointer fallback
if (localMatrix) {
    return sk_make_sp<SkLocalMatrixShader>(sk_sp<SkShader>(std::move(t)), *localMatrix);
} else {
    return sk_make_sp<SkLocalMatrixShader>(sk_sp<SkShader>(std::move(t)), SkMatrix::I());
}
```

Good:

```cpp
// Good: Hoist common assertions and argument pushes upfront before branching
SkASSERT(arg0.type().matches(arg1.type()));
if (!this->pushExpression(arg0) || !this->pushExpression(arg1)) {
    return unsupported();
}
if (arg2.type().componentType().isFloat()) {
    // ...
}
if (arg2.type().componentType().isBoolean()) {
    if (!this->pushExpression(arg2)) {
        return unsupported();
    }
    // ...
}

// Good: Isolate differing argument with ternary and hoist common function call
auto asset = fResourceProvider->loadImageAsset(
        path->begin(), name->begin(), slotID ? slotID->begin() : id->begin());

// Good: Isolate optional pointer fallback in a single return statement
return sk_make_sp<SkLocalMatrixShader>(
        sk_sp<SkShader>(std::move(t)), localMatrix ? *localMatrix : SkMatrix::I());
```

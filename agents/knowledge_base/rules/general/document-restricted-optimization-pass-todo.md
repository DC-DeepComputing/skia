---
name: skia/document-restricted-optimization-pass-todo
description: When introducing partial implementations, unhandled switch cases, hardcoded checks or defaults, or stopgap shortcuts during incremental development, document deferred work with an attributed TODO
rubric: readability
severity: should_fix
---

**Rule.** When implementing partial features, decoupling backends, or
scaffolding new subsystems during incremental development—such as omitting
struct layout validation, leaving switch cases unhandled, introducing restricted
passes evaluating only root nodes, hardcoding types, dimensions, identifier
lists, or fallback option defaults, capturing static assets, or adopting
temporary copy shortcuts—document the limitation, historical context, and
deferred generalization with an attributed TODO(b/...) or TODO(bug) comment.
Explicitly identify the unhandled variants, missing validation, fragile stopgap
checks, or planned parameter/capability-flag refactoring so reviewers and future
maintainers recognize the code as an intentional stopgap rather than an
accidental oversight.

**Why.** Omitting switch cases, deferring layout validation, hardcoding checks
or fallback defaults, or relying on temporary shortcuts appears
indistinguishable from an unintended oversight or permanent architectural
limitation unless documented. Explaining the historical context and attaching an
attributed TODO preserves design intent, clarifies why proper parameter
plumbing, capability flags, or generalization was deferred, and tracks technical
debt for follow-up work.

## Examples

Bad:

```cpp
// Hardcoding option defaults as a decoupling shortcut without explaining context:
constexpr float kMinDistanceFieldFontSize = 18.f;

// Omitting remaining switch cases during incremental development without explanation:
switch (shapeType) {
    case $kShapeTypeRect:
        return $rect_blur_coverage_fn(coords, shapeData, isFast, invSixSigma, s);
}
return half4(0);

// Hardcoding checks against specific IDs as a stopgap instead of checking capability flags:
if (outDescs && (snippetId == BuiltInCodeSnippetID::kImageShader ||
                 snippetId == BuiltInCodeSnippetID::kHWImageShader)) {
    append_sampler_descs(node->data(), *outDescs);
}
```

Good:

```cpp
// Documents historical context and adds TODO to wire configurable parameters across backends:
// TODO(herb, jvanverth): pipe in parameters that can be used for both Ganesh and Graphite.
constexpr float kMinDistanceFieldFontSize = 18.f;

// Attributed TODO documenting unhandled switch cases deferred to tracking bug:
switch (shapeType) {
    case $kShapeTypeRect:
        return $rect_blur_coverage_fn(coords, shapeData, isFast, invSixSigma, s);
    // TODO(b/238762890): Handle $kShapeTypeRRect and $kShapeTypeCircle.
}
return half4(0);

// Attributed TODO tracking planned refactoring to replace fragile stopgap with capability flag:
// TODO(b/369846881): Refactor checking snippet ID to named snippet requirement flag to decrease fragility.
if (outDescs && (snippetId == BuiltInCodeSnippetID::kImageShader ||
                 snippetId == BuiltInCodeSnippetID::kHWImageShader)) {
    append_sampler_descs(node->data(), *outDescs);
}
```

---
name: skia/sync-inline-comments-with-expanded-conditions
description: When documenting code logic, operands, boundaries, or fallbacks, keep explanatory comments synchronized with active semantics and remove obsolete or contradictory notes
rubric: readability
severity: should_fix
---

**Rule.** When documenting code logic, operands, boundaries, conditional guards,
or fallbacks, ensure explanatory comments stay synchronized with active code
semantics and units. Accurately reflect abstraction levels (e.g., distinguishing
expressions from variables, or code units from graphemes) and operational
directions rather than contradicting the implementation. Update comments to
reflect all handled variants rather than a narrower subset, accurately describe
contrasting branches and restored pipeline stages, and remove obsolete or
speculative WIP notes.

**Why.** Inaccurate, stale, or speculative comments that contradict code
semantics, misidentify abstraction levels or units, invert operational
directions, omit newly supported cases, or describe superseded constructs and
deleted fallbacks mislead maintainers about active behavior and safety
invariants. Keeping comments synchronized with actual code and removing obsolete
notes prevents confusion over active preconditions, supported types, and
execution pipelines.

## Examples

Bad:

```cpp
// 1) Comment contradicts operation direction and conflates abstraction levels:
// Shift to the left if it's inside a grapheme
auto utf8 = fUTF8IndexForUTF16Index[start];
if (start > 0 && fUTF8IndexForUTF16Index[start - 1] == utf8) {
    ++utf8;
}

// 2) Stale comment claims paintID is always attached:
GraphicsPipelineCache::Index idx = fPipelineCache.insert(
        {step->renderStepID(), paintID});

// 3) Comment describes narrower subset after generalization:
// Can specialize on arguments that are a variable reference.
if (arg.is<VariableReference>()) {
    argBase = arg.as<VariableReference>().variable();
}
```

Good:

```cpp
// 1) Comment accurately describes operation, units, and encoding boundary:
auto utf8 = fUTF8IndexForUTF16Index[start];
// If utf8 is not at the first code unit of a utf-16 encoded codepoint then increment it
if (start > 0 && fUTF8IndexForUTF16Index[start - 1] == utf8) {
    ++utf8;
}

// 2) Comment updated or removed to reflect conditional parameter binding:
GraphicsPipelineCache::Index idx = fPipelineCache.insert(
        {step->renderStepID(), step->performsShading() ? paintID : UniquePaintParamsID::Invalid()});

// 3) Inline comment reflects all handled variants and abstraction levels:
// Can specialize on arguments that are a variable reference or field access.
if (arg.is<VariableReference>()) {
    argBase = arg.as<VariableReference>().variable();
} else if (arg.is<FieldAccess>() &&
           arg.as<FieldAccess>().base()->is<VariableReference>()) {
    argBase = arg.as<FieldAccess>().base()->as<VariableReference>().variable();
}
```

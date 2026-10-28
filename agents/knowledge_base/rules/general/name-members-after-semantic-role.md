---
name: skia/name-members-after-semantic-role
description: When declaring member fields, configuration options, parameters, or local variables, name them after concrete domain roles rather than generic placeholders or base types
rubric: readability
severity: should_fix
---

**Rule.** Name member fields, configuration options, parameters, and local
variables after their specific domain role, companion member, or provenance
rather than generic placeholders or base types (e.g., 'argBase' instead of vague
'var', 'bufferForLargerValues' over generic 'buffer' or 'stream',
'fInternalMSAATileSize' over 'fInternalMultisampleDimsLimit', 'fBindingList'
over 'fWrapper', and 'fContentSize' matching 'fContentShader' instead of
ambiguous 'fChildSize'). When multiple buffers or streams coexist in the same
scope, distinguish auxiliary containers by their content or role. Furthermore,
align names with companion members and maintain consistent terminology across
parallel scalar and SIMD implementations (such as 'sortedXDir' rather than
generic 'xDir').

**Why.** Naming fields, parameters, or variables after vague abstractions, base
types, or generic terms like 'var', 'buffer', 'stream', or 'child' obscures
their concrete role and creates ambiguity when multiple related entities or
streams exist in scope. Similarly, generic names like 'params' or 'xDir' lack
domain context. Using domain-specific identifiers aligned with companion
members, stream contents, and parallel counterparts clarifies intent and makes
data flow intuitive.

## Examples

Bad:

```cpp
// Generic placeholder / base type instead of role or provenance:
const Variable* var = nullptr;
if (arg.is<VariableReference>()) {
    var = arg.as<VariableReference>().variable();
}
struct Insertion { BindingWrapper* fWrapper; };

// Generic buffer/stream name obscuring auxiliary role:
SkDynamicMemoryWStream stream;
SkDynamicMemoryWStream buffer;

// Ambiguous generic child name when companion is fContentShader:
class SkSLShaderNode final : public sksg::CustomRenderNode {
    SkSLShaderNode(sk_sp<RenderNode> child, const SkSize& child_size)
        : INHERITED({std::move(child)}), fChildSize(child_size) {}
    sk_sp<SkShader> fContentShader;
    const SkSize fChildSize;
};

// Generic parameter / inconsistent naming in SIMD counterpart:
LineStepParams params = this->computeLineStepParams(pTop, pBot, dx, dy);
bool xDir = pTop.fX <= pBot.fX;
```

Good:

```cpp
// Concrete role and provenance (argBase) rather than generic 'var':
const Variable* argBase = nullptr;
if (arg.is<VariableReference>()) {
    argBase = arg.as<VariableReference>().variable();
}
struct Insertion { BindingWrapper* fBindingList; };

// Descriptive name reflecting specific auxiliary role or content:
SkDynamicMemoryWStream stream;
SkDynamicMemoryWStream bufferForLargerValues;

// Aligned with companion member (fContentSize matching fContentShader):
class SkSLShaderNode final : public sksg::CustomRenderNode {
    SkSLShaderNode(sk_sp<RenderNode> child, const SkSize& content_size)
        : INHERITED({std::move(child)}), fContentSize(content_size) {}
    sk_sp<SkShader> fContentShader;
    const SkSize fContentSize;
};

// Descriptive parameter / matching scalar counterpart's fSortedXDir:
LineStepParams stepParams = this->computeLineStepParams(pTop, pBot, dx, dy);
bool fSortedXDir = pTop.fX <= pBot.fX;
```

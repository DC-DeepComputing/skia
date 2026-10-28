---
name: skia/modules/resolve-runtime-effect-children-by-name
description: When binding inputs to runtime effect shaders in effect adapters, resolve child indices dynamically by name instead of injecting synthetic declarations or hardcoding slots
rubric: design
severity: should_fix
---

**Rule.** Do not inject synthetic uniform declarations into user runtime shader
source or hardcode fixed child index positions. Model input sources (such as
images and layer content) with explicit property types and resolve child shader
slot indices dynamically by name using SkRuntimeEffect::findChild().

**Why.** Prepending synthetic uniform declarations and hardcoding child indices
makes brittle assumptions about shader source structure and child declaration
order. Dynamic lookup by uniform name allows shaders to declare inputs in any
order and cleanly separates property types from shader compilation.

## Examples

Bad:

```cpp
// Prepending implicit uniform and assuming fixed slot 0
shader.prepend(std::string_view("uniform shader u_layer;"));
...
fChildren.at(0) = this->node()->contentShader();
```

Good:

```cpp
// Distinguish property types and resolve child indices by uniform name
} else if (type == kSkSLProp_layer) {
    fChildren.push_back({type, SkString(uniformName->begin(), uniformName->size()), nullptr});
}
...
auto metadata = fEffect->findChild(childData.name.c_str());
if (childData.type == kSkSLProp_layer && metadata) {
    childrenData[metadata->index] = node->contentShader();
}
```

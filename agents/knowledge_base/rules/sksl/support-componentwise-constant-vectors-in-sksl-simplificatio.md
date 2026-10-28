---
name: skia/sksl/support-componentwise-constant-vectors-in-sksl-simplificatio
description: When simplifying or strength-reducing vector constant expressions in SkSL, evaluate and transform each vector slot individually instead of requiring uniform splats
rubric: correctness
severity: should_fix
---

**Rule.** In SkSL constant folding and expression simplifications, support
general component-wise constant vectors rather than requiring splatted or
uniform values across all lanes. Inspect and transform each component slot
individually via type().slotCount() and getConstantValue(index) when all
components satisfy the operation's validity criteria, reconstructing the result
with ConstructorCompound::Make or ConstructorCompound::MakeFromConstants.

**Why.** Vector operations such as division by a constant vector (e.g. v /
vec3(1, 2, 4)) can be strength-reduced into multiplication by reciprocal
constants component-wise just as effectively as uniform splats (e.g. v / vec3(2,
2, 2)). Requiring all vector lanes to share identical values misses optimization
opportunities on non-uniform compile-time constant vectors.

## Examples

Bad:

```cpp
// Restricts simplification to uniform splat vectors where all lanes match
std::optional<double> value = is_any_constant_value(right);
if (!value || !std::isfinite(*value) || *value == 0.0f) {
    return nullptr;
}
Literal reciprocal{right.fPosition, 1.0 / *value, &right.type().componentType()};
return cast_expression(context, reciprocal.fPosition, reciprocal, right.type());
```

Good:

```cpp
// Inspects and transforms each vector component slot independently
int nslots = right.type().slotCount();
SkSTArray<4, double> values;
for (int index = 0; index < nslots; ++index) {
    std::optional<double> value = right.getConstantValue(index);
    if (!value || !std::isfinite(*value) || *value == 0.0f) {
        return nullptr;
    }
    values.push_back(1.0 / *value);
}
return ConstructorCompound::MakeFromConstants(context, right.fPosition, right.type(), values);
```

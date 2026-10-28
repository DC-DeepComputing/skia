---
name: skia/api/order-struct-member-comparisons-by-declaration
description: When implementing struct equality operators or equivalence helper functions, evaluate member fields in the exact order they are declared in the struct definition
rubric: readability
severity: nit
---

**Rule.** When implementing operator==, operator!=, or struct equivalence helper
functions, compare member fields in the exact sequential order they are declared
in the struct or class definition. Do not evaluate members out of order or
transpose adjacent fields.

**Why.** Evaluating fields in declaration order makes comparison routines
predictable, intuitive to audit against struct header definitions, and easier to
maintain. It minimizes the risk of accidentally omitting newly added members or
redundantly checking the same field twice.

## Examples

Bad:

```cpp
bool DawnDescriptorsAreEquivalent(const wgpu::YCbCrVkDescriptor& desc1,
                                  const wgpu::YCbCrVkDescriptor& desc2) {
    // vkYCbCrModel is compared before vkFormat despite being declared after it in the struct.
    return desc1.vkYCbCrModel            == desc2.vkYCbCrModel            &&
           desc1.vkFormat                == desc2.vkFormat                &&
           desc1.vkYCbCrRange            == desc2.vkYCbCrRange;
}
```

Good:

```cpp
bool DawnDescriptorsAreEquivalent(const wgpu::YCbCrVkDescriptor& desc1,
                                  const wgpu::YCbCrVkDescriptor& desc2) {
    // Fields are evaluated in the exact order they are declared in the struct definition.
    return desc1.vkFormat                == desc2.vkFormat                &&
           desc1.vkYCbCrModel            == desc2.vkYCbCrModel            &&
           desc1.vkYCbCrRange            == desc2.vkYCbCrRange;
}
```

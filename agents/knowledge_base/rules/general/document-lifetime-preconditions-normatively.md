---
name: skia/document-lifetime-preconditions-normatively
description: When documenting lifetime requirements or preconditions in comments, use normative language ('must outlive' or 'must stay in scope') rather than advisory phrasing ('should')
rubric: readability
severity: nit
---

**Rule.** When documenting lifetime requirements or preconditions in code
comments (such as for chained Vulkan structs, raw pointers, or non-owning
references), use normative phrasing ('must stay in scope' or 'must outlive')
rather than advisory language ('should'). In addition, explain how the design
preserves the required lifetime (for example, by identifying the containing
struct, owner, or passing pattern that guarantees it).

**Why.** Lifetime requirements in C++ are mandatory safety contracts, not
optional recommendations. Advisory phrasing like 'should be in scope' can
mislead callers into treating mandatory invariants as optional suggestions,
risking use-after-free bugs or memory corruption. Normative language
unambiguously communicates the precondition, while explaining the architectural
mechanism clarifies why the code is structured to satisfy it.

## Examples

Bad:

```cpp
// Note: Any struct that setup_features chains should be in scope until vkCreateDevice.
void setup_feature_query(TestVkFeatures& testFeatures, bool isProtected);
```

Good:

```cpp
// Note: Any struct chained onto the feature query must stay in scope until vkCreateDevice.
// This is why these structs are located in TestVkFeatures and passed into this function.
void setup_feature_query(TestVkFeatures& testFeatures, bool isProtected);
```
